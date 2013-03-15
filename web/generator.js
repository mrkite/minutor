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

/**
 * @constructor
 * @param {Mods} mods
 * @param {Display} display
 */
function PackGenerator(mods,display)
{
	/** @type {Object.<string,Object>} */
	var pack;
	/** @type {Object} */
	var packInfo;

	/**
	 * @param {string} key
	 * @param {boolean} islocal
	 */
	this.generate=function(key,islocal)
	{
		pack={'pack_info.json':{'type':'pack','data':[]}};
		packInfo=pack['pack_info.json'];
		if (islocal)
		{
			var p=mods.localPacks[key];
			packInfo['name']=p['name'];
			packInfo['version']=p['version'];
			if (p['update']!=undefined)
				packInfo['update']=p['update'];
			for (var i=0;i<p['data'].length;i++)
			{
				var m;
				if (p['data'][i]['type']=='pre')
					m=mods.mods[p['data'][i]['id']];
				else
					m=mods.locals[p['data'][i]['id']];
				var fn=makeFilename(m['name'],m['type']);
				packInfo['data'].push(fn);
				pack[fn]=m;
			}
		}
		else
		{
			var p=mods.packs[key];
			packInfo['name']=p['name'];
			packInfo['version']=p['version'];
			for (var i=0;i<p['data'].length;i++)
			{
				var m=mods.mods[p['data'][i]];
				var fn=makeFilename(m['name'],m['type']);
				packInfo['data'].push(fn);
				pack[fn]=m;
			}
		}

		//now check to see if any mods require configuration
		var needsConfig=false;
		for (var i=0;i<packInfo['data'].length;i++)
		{
			if (pack[packInfo['data'][i]]['configs']!=undefined)
			{
				needsConfig=true;
				break;
			}
		}
		if (needsConfig)
		{
			var bg=display.toDOM({'div':null,'class':'dropbg'});
			var msg=display.toDOM({'div':['Drop config folder here',
				{'div':"Or click to cancel"}],'class':'dropmsg'});
			var dropbox=display.toDOM({'div':null,'class':'dropbox'});
			display.draw([bg,msg,dropbox]);
			dropbox.click(function(){
				dropbox.remove();
				msg.remove();
				bg.remove();
			});
			dropbox.bind({
				'dragover': function() {
					bg.addClass('hover');
					return false;
				},
				'dragleave': function() {
					bg.removeClass('hover');
					return false;
				},
				'dragend': function() {
					bg.removeClass('hover');
					return false;
				},
				'drop': function(e) {
					dropbox.remove();
					msg.remove();
					bg.remove();
					try {
						var item=e.originalEvent.dataTransfer['items'][0];
						if (item['getAsEntry'])
							readConfigs(item['getAsEntry']());
						else if (item['webkitGetAsEntry'])
							readConfigs(item['webkitGetAsEntry']());
						else
							throw "Not supported";
					} catch (err) {
						alert(err);
					}
					return false;
				}
			});
		}
		else
			zipPack();
	}

	/** @type {number} */
	var waitingForCfgs=0;

	/** @param {FileEntry} entry */
	function readConfigs(entry)
	{
		if (!entry.isDirectory)
			return;
		waitingForCfgs=0;
		for (var mod in pack)
		{
			if (pack[mod]['configs']!=undefined)
				applyConfigs(entry,mod);
		}
	}

	/**
	 * @param {FileEntry} entry
	 * @param {string} key
	 */
	function applyConfigs(entry,key)
	{
		//current mod is now a template
		var template=pack[key];
		var mod={'name':template['name'],
			'type':template['type'],
			'version':template['version'],
			'data':[]};
		//and this is the new mod, with configs applied
		pack[key]=mod;

		for (var i=0;i<template['data'].length;i++)
		{
			//clone into pack
			mod['data'].push(jQuery.extend(true,{},template['data'][i]));
		}

		// load and parse each config
		for (var i=0;i<template['configs'].length;i++)
		{
			waitingForCfgs++;
			loadConfig(entry,template['configs'][i],mod);
		}
	}

	/**
	 * @param {FileEntry} entry
	 * @param {string} name
	 * @param {Object} mod
	 */
	function loadConfig(entry,name,mod)
	{
		entry['getFile'](name,{},function(file) {
			file.file(function(f) {
				var reader=new FileReader();
				reader.onloadend=function(e) {
					var cfg=parseConfig(this.result);
					for (var m=0;m<mod['data'].length;m++)
					{
						if (mod['data'][m]['config'] && cfg[mod['data'][m]['config']]!=undefined)
						{
							mod['data'][m]['id']=cfg[mod['data'][m]['config']];
							delete mod['data'][m]['config'];
						}
					}
					waitingForCfgs--;
					if (waitingForCfgs==0) // no more configs anywhere
						zipPack();
				}
				reader.onerror=function(e) {
					window.console.log("error");
					window.console.log(e);
				}
				reader.readAsText(f);
			});
		},function(err) {
				alert("Couldn't find "+name);
		});
	}

	/**
	 * @param {string} cfg
	 * @returns {Object.<string,number>}
	 */
	function parseConfig(cfg)
	{
		var hash={};
		var prefix=[];
		var pendingsymbol='';
		var lines=cfg.split(/\n|\r/);
		for (var i=0;i<lines.length;i++)
		{
			var line=lines[i];
			if (line.match(/#/))
					line=line.replace(/#.*/,'');
			line=line.trim();
			if (line=='')
				continue;
			if (line.match(/,/))
			{
				var parts=line.split(/,/);
				for (var j=0;j<parts.length;j++)
					addConfigLine(prefix,hash,parts[j]);
			}
			else if (line.match(/=/))
				addConfigLine(prefix,hash,line);
			else if (line.match(/{/))
			{
				if (pendingsymbol=='')
					prefix.push(toSymbol(line.match(/^(.+){/)[1]));
				else
					prefix.push(pendingsymbol);
				pendingsymbol='';
			}
			else if (line.match(/}/))
				prefix.pop();
			else if (line.match(/\w/))
				pendingsymbol=toSymbol(line);
			else
				throw "unknown:"+line;
		}
		return hash;
	}
	/**
	 * @param {Array.<string>} prefix
	 * @param {Object.<string,number>} hash
	 * @param {string} line
	 */
	function addConfigLine(prefix,hash,line)
	{
		if (!line.match(/=/))
			return;
		var parts=line.split(/=/);
		var key=parts[0];
		if (key.match(/:/))
		{
			var kparts=key.split(/:/);
			key=kparts[1];
		}
		prefix.push(toSymbol(key));
		var val=toSymbol(parts[1]);
		if ($.isNumeric(val))
			hash[prefix.join('.')]=parseInt(val,10);
		prefix.pop();
	}
	/**
	 * @param {string} sym
	 * @returns {string}
	 */
	function toSymbol(sym)
	{
		sym=sym.trim();
		while (sym.match(/"/))
			sym=sym.replace(/^"(.*)"$/,'$1');
		return sym;
	}

	/**
	 * @param {string} name
	 * @param {string} type
	 * @returns {string}
	 */
	function makeFilename(name,type)
	{
		var pre=name.toLowerCase().replace(/[^a-z0-9]/g,'');
		var post;
		switch (type)
		{
			case 'block': post="_ids.json"; break;
			case 'biome': post="_biomes.json"; break;
			case 'dimension': post="_dims.json"; break;
		}
		var fn=pre+post;
		var mid=1;
		while (fn in pack)
		{
			mid++;
			fn=pre+mid+post;
		}
		return fn;
	}

	function zipPack()
	{
		// remove any unconfigured lines from the pack
		// we do this because the pack might be using an older config
		for (var f in pack)
		{
			if (pack[f]['type']=='pack') continue;
			for (var i=pack[f]['data'].length-1;i>=0;i--)
				if (pack[f]['data'][i]['config']!==undefined)
					pack[f]['data'].splice(i,1); //remove it
			if (pack[f]['data'].length==0) //no data?  remove entire mod
			{
				delete pack[f];
				for (var i=0;i<packInfo['data'].length;i++)
					if (packInfo['data'][i]==f)
					{
						packInfo['data'].splice(i,1); //and remove reference
						break;
					}
			}
		}
		var files=[];
		for (var f in pack)
			files.push({name:f,file:window.JSON.stringify(pack[f])});
		var zip=Zip(files);
		var fn=packInfo['name'].toLowerCase().replace(/[^a-z0-9]/g,'')+'.zip';

		var blob=new Blob([zip],{'type':'application/zip'});
		var url=window.URL.createObjectURL(blob);
		var savelink=$(document.createElement('a'));
		savelink.attr('href',url);
		savelink.attr('download',fn);
		var event=document.createEvent("MouseEvents");
		event.initMouseEvent("click",true,false,window,1,0,0,0,0,
				false,false,false,0,null);
		savelink[0].dispatchEvent(event);
		setTimeout(function() {
			window.URL.revokeObjectURL(url);
		},5000); //5 seconds then nuke url
	}
}
