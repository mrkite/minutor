/*
   Copyright (c) 2013, Sean Kasun
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "zipreader.h"
#include "zlib.h"

ZipReader::ZipReader(const QString filename) : f(filename)
{
}

bool ZipReader::open()
{
	if (!f.open(QIODevice::ReadOnly))
		return false;

	//locate end of central directory record
	qint64 ziplen=f.size();
	qint64 maxECDlen=0xffff+22; //max comment len + ECD
	if (maxECDlen>ziplen) //zip is shorter?
		maxECDlen=ziplen;

	f.seek(ziplen-maxECDlen); //ECD must be after this

	QByteArray data=f.read(maxECDlen);
	const quint8 *p=(const quint8 *)data.constData();

	bool found=false;
	//now scan this data for the ECD signature
	for (qint64 i=0;i<maxECDlen-20;i++,p++)
	{
		if (p[0]==0x50 && p[1]==0x4b && p[2]==0x05 && p[3]==0x06)
		{
			found=true;
			break;
		}
	}
	if (!found) //no ecd found, probably not a zip
	{
		f.close();
		return false;
	}

	//awesome, now to find the central directory
	qint32 cdnum=p[0xa]|(p[0xb]<<8);
	qint32 cdsize=p[0xc]|(p[0xd]<<8)|(p[0xe]<<16)|(p[0xf]<<24);
	qint64 cdoffset=p[0x10]|(p[0x11]<<8)|(p[0x12]<<16)|(p[0x13]<<24);

	f.seek(cdoffset);
	data=f.read(cdsize);
	p=(const quint8 *)data.constData();

	//loop through all the files
	for (int i=0;i<cdnum;i++)
	{
		if (p[0]!=0x50 || p[1]!=0x4b || p[2]!=0x01 || p[3]!=0x02) // not a central file header?
		{
			f.close(); //screw this zip
			return false;
		}
		qint16 namelen=p[0x1c]|(p[0x1d]<<8);
		qint16 extralen=p[0x1e]|(p[0x1f]<<8);
		qint16 commentlen=p[0x20]|(p[0x21]<<8);
		QString key=QString::fromUtf8((char *)p+0x2e,namelen);
		ZipFileHeader &zfh=files[key];
		zfh.compressed=p[0x14]|(p[0x15]<<8)|(p[0x16]<<16)|(p[0x17]<<24);
		zfh.uncompressed=p[0x18]|(p[0x19]<<8)|(p[0x1a]<<16)|(p[0x1b]<<24);
		zfh.compression=p[0xa]|(p[0xb]<<8);
		zfh.offset=p[0x2a]|(p[0x2b]<<8)|(p[0x2c]<<16)|(p[0x2d]<<24);
		p+=0x2e +namelen+extralen+commentlen; //next file
	}
	return true;
}
QByteArray ZipReader::get(const QString filename)
{
	if (!files.contains(filename))
		return QByteArray();
	ZipFileHeader &zfh=files[filename];
	if (zfh.compression!=0 && zfh.compression!=8) //unsupported compression
		return QByteArray();
	f.seek(zfh.offset);
	QByteArray lfh=f.read(0x1e); //read local file header
	const quint8 *p=(const quint8 *)lfh.constData();

	//make sure we got a local file header
	if (p[0]!=0x50 || p[1]!=0x4b || p[2]!=0x03 || p[3]!=0x04)
		return QByteArray();

	qint32 namelen=p[0x1a]|(p[0x1b]<<8);
	qint32 extralen=p[0x1c]|(p[0x1d]<<8);
	f.seek(zfh.offset+namelen+extralen+0x1e); //skip header

	QByteArray comp=f.read(zfh.compressed);
	if (zfh.compression==0) //no compression
		return comp;
	QByteArray result;

	z_stream strm;
	static const int CHUNK_SIZE = 8192;
	char out[CHUNK_SIZE];
	strm.zalloc=Z_NULL;
	strm.zfree=Z_NULL;
	strm.opaque=Z_NULL;
	strm.avail_in=comp.size();
	strm.next_in=(Bytef*)comp.data();

	inflateInit2(&strm,-MAX_WBITS);
	do {
		strm.avail_out=CHUNK_SIZE;
		strm.next_out=(Bytef*)out;
		inflate(&strm,Z_NO_FLUSH);
		result.append(out,CHUNK_SIZE-strm.avail_out);
	} while (strm.avail_out==0);
	inflateEnd(&strm);
	return result;
}
void ZipReader::close()
{
	files.clear(); //erase all headers
	f.close();
}
