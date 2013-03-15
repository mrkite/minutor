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

/*
Lightweight zip function

"zips" up a bunch of text files.  Doesn't actually compress the files since
the files we're dealing with are so small it doesn't make much of a
difference... and therefore we don't have to implement RFC1951 (yay).

files is an array of {name:"filename",file:"text contents of file"}
*/

/**
 * @param {Array.<{name:string,file:string}>} files
 * @returns {Uint8Array}
 */
function Zip(files)
{
	var crctable=[];
	for (var i=0;i<256;i++)
	{
		var c=i;
		for (var j=0;j<8;j++)
			if (c&1)
				c=(c>>>1)^0xedb88320;
			else
				c=(c>>>1);
		crctable[i]=c;
	}
	function crc32(msg)
	{
		var c=-1;
		for (var i=0;i<msg.length;i++)
			c=(c>>>8)^crctable[(c^msg.charCodeAt(i))&0xff];
		return ~c;
	}

	var date=new Date();
	var dosdate=((date.getFullYear()-1980)<<9)|((date.getMonth()+1)<<5)|date.getDate();
	var crcs=[];
	var data=[];

	var offset=0;
	//local files
	for (var i=0;i<files.length;i++)
	{
		files[i].offset=offset;
		//local file header signature
		data[offset++]=0x50; data[offset++]=0x4b;
		data[offset++]=0x03; data[offset++]=0x04;
		//version needed to extract
		data[offset++]=20; data[offset++]=0;
		//general purpose bit flag
		data[offset++]=0; data[offset++]=0;
		//compression method
		data[offset++]=0; data[offset++]=0; //no compression
		//last mod file time
		data[offset++]=0; data[offset++]=0;
		//last mod file date
		data[offset++]=dosdate&0xff; data[offset++]=(dosdate>>8)&0xff;
		//crc32
		crcs[i]=crc32(files[i].file);
		data[offset++]=crcs[i]&0xff; data[offset++]=(crcs[i]>>8)&0xff;
		data[offset++]=(crcs[i]>>16)&0xff; data[offset++]=(crcs[i]>>24)&0xff;
		//compressed size
		var size=files[i].file.length;
		data[offset++]=size&0xff; data[offset++]=(size>>8)&0xff;
		data[offset++]=(size>>16)&0xff; data[offset++]=(size>>24)&0xff;
		//uncompressed size
		data[offset++]=size&0xff; data[offset++]=(size>>8)&0xff;
		data[offset++]=(size>>16)&0xff; data[offset++]=(size>>24)&0xff;
		//file name length
		data[offset++]=files[i].name.length; data[offset++]=0;
		//extra field length
		data[offset++]=0; data[offset++]=0;
		//file name
		for (var j=0;j<files[i].name.length;j++)
			data[offset++]=files[i].name.charCodeAt(j);
		//extra field not present
		//file data
		for (var j=0;j<size;j++)
			data[offset++]=files[i].file.charCodeAt(j);
	}
	//central directory header
	var cdstart=offset;
	for (var i=0;i<files.length;i++)
	{
		//central file header signature
		data[offset++]=0x50; data[offset++]=0x4b;
		data[offset++]=0x01; data[offset++]=0x02;
		//version made by
		data[offset++]=20; data[offset++]=0;
		//version needed to extract
		data[offset++]=20; data[offset++]=0;
		//general purpose bit flag
		data[offset++]=0; data[offset++]=0;
		//compression method
		data[offset++]=0; data[offset++]=0;
		//last mod file time
		data[offset++]=0; data[offset++]=0;
		//last mod file date
		data[offset++]=dosdate&0xff; data[offset++]=(dosdate>>8)&0xff;
		//crc-32
		data[offset++]=crcs[i]&0xff; data[offset++]=(crcs[i]>>8)&0xff;
		data[offset++]=(crcs[i]>>16)&0xff; data[offset++]=(crcs[i]>>24)&0xff;
		//compressed size
		var size=files[i].file.length;
		data[offset++]=size&0xff; data[offset++]=(size>>8)&0xff;
		data[offset++]=(size>>16)&0xff; data[offset++]=(size>>24)&0xff;
		//uncompressed size
		data[offset++]=size&0xff; data[offset++]=(size>>8)&0xff;
		data[offset++]=(size>>16)&0xff; data[offset++]=(size>>24)&0xff;
		//file name length
		data[offset++]=files[i].name.length; data[offset++]=0;
		//extra field length
		data[offset++]=0; data[offset++]=0;
		//file comment length
		data[offset++]=0; data[offset++]=0;
		//disk number start
		data[offset++]=0; data[offset++]=0;
		//internal file attributes
		data[offset++]=0; data[offset++]=0;
		//external file attributes
		data[offset++]=0; data[offset++]=0;
		data[offset++]=0; data[offset++]=0;
		//relative offset of local header
		var ofs=files[i].offset;
		data[offset++]=ofs&0xff; data[offset++]=(ofs>>8)&0xff;
		data[offset++]=(ofs>>16)&0xff; data[offset++]=(ofs>>24)&0xff;
		//filename
		for (var j=0;j<files[i].name.length;j++)
			data[offset++]=files[i].name.charCodeAt(j);
		//extra field is blank
		//file comment is blank
	}
	var cdlen=offset-cdstart;
	//end of central directory record
	//end of central dir signature
	data[offset++]=0x50; data[offset++]=0x4b;
	data[offset++]=0x05; data[offset++]=0x06;
	//disk number
	data[offset++]=0; data[offset++]=0;
	//central directory starting disk
	data[offset++]=0; data[offset++]=0;
	//number of entries on this disk
	var num=files.length;
	data[offset++]=num&0xff; data[offset++]=(num>>8)&0xff;
	//number of entries on all disks
	data[offset++]=num&0xff; data[offset++]=(num>>8)&0xff;
	//size of central directory
	data[offset++]=cdlen&0xff; data[offset++]=(cdlen>>8)&0xff;
	data[offset++]=(cdlen>>16)&0xff; data[offset++]=(cdlen>>24)&0xff;
	//central directory start offset
	data[offset++]=cdstart&0xff; data[offset++]=(cdstart>>8)&0xff;
	data[offset++]=(cdstart>>16)&0xff; data[offset++]=(cdstart>>24)&0xff;
	//comment length
	data[offset++]=0; data[offset++]=0;
	//zip comment is blank

	return new Uint8Array(data);
}
