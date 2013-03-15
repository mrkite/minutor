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
 * @param {Function} updatecb
 * @param {Display} display
 */
function Mods(updatecb,display)
{
	/** @type {Object.<string,Object>} */
	var mods={};
	this.mods=mods;
	/** @type {Array.<Object>} */
	var packs=[];
	this.packs=packs;
	/** @type {Object.<string,Object>} */
	var locals={}
	this.locals=locals;
	/** @type {Array.<Object>} */
	var localPacks=[];
	this.localPacks=localPacks;

	/** @type {number} */
	var loading=1;

	$.getJSON("mods/mods.json",function(m) {
		loading+=m.length;
		for (var i=0;i<m.length;i++)
		{
			mods[m[i]]=null;
			load(m[i]);
		}
	});
	$.getJSON("mods/packs.json",function(p) {
		for (var i=0;i<p.length;i++)
			packs.push(p[i]);
		loading--;
		if (!loading)
			updatecb();
	});

	loadLocals();
	loadLocalPacks();

	/** @param {string} mod */
	function load(mod)
	{
		$.getJSON("mods/"+mod,function(m) {
			mods[mod]=m;
			loading--;
			if (!loading)
				updatecb();
		});
	}

	function loadLocals()
	{
		for (var m in locals)
			delete locals[m];
		var lms=window.localStorage.getItem('localmods');
		if (lms!=null)
		{
			var ls=window.JSON.parse(String(lms));
			for (var m in ls)
				locals[m]=ls[m];
		}
	}
	function saveLocals()
	{
		window.localStorage.setItem('localmods',window.JSON.stringify(locals));
	}
	function loadLocalPacks()
	{
		localPacks.length=0;
		var lps=window.localStorage.getItem('localpacks');
		if (lps!=null)
		{
			var ls=window.JSON.parse(String(lps));
			for (var i=0;i<ls.length;i++)
				localPacks.push(ls[i]);
		}
	}
	function saveLocalPacks()
	{
		window.localStorage.setItem('localpacks',window.JSON.stringify(localPacks));
	}

	function addLocal(mod)
	{
		var pre=mod['name'].toLowerCase().replace(/[^a-z0-9]/g,'');
		var post;
		if (mod['type']=='block')
			post='_ids.json';
		else if (mod['type']=='biome')
			post='_biomes.json';
		else if (mod['type']=='dimension')
			post='_dims.json';
		var fn=pre+post;
		var mid=1;
		while (fn in locals)
		{
			mid++;
			fn=pre+mid+post;
		}
		locals[fn]=mod;
		saveLocals();
	}

	this.add=function()
	{
		var dialog=new Dialog(display,['Create','Cancel'],addResponse);

		var types=[
			{'option':'New mod blocks','val':'-1'},
			{'option':'New mod biomes','val':'-2'},
			{'option':'New mod dimensions','val':'-3'}
		];
		for (var mod in mods)
			types.push({'option':'Copy '+mods[mod]['name']+' ('+mods[mod]['type']+')','val':mod});

		var nmext=display.toDOM({'select':types});
		var nmname=display.toDOM({'input':null,'type':'text','size':'30'});
		var drop=display.toDOM({'div':"or drop an exported mod here",'class':'droptarget'});
		dialog.draw([
			{'h4':'Add Mod'},
			{'dl':[
				{'dt':'Type:'},
				{'dd':nmext},
				{'dt':'Name:'},
				{'dd':nmname}]},
			drop
			]);
		nmname.focus();
		drop.bind({
			'dragover': function() {
				drop.addClass('active');
				return false;
			},
			'dragleave': function() {
				drop.removeClass('active');
				return false;
			},
			'dragend': function() {
				drop.removeClass('active');
				return false;
			},
			'drop': function(e) {
				try {
					var blob=e.originalEvent.dataTransfer['items'][0]['getAsFile']();
					var reader=new FileReader();
					reader.onload=function(event){
						dialog.close();
						addLocal(window.JSON.parse(event.target.result));
						updatecb();
					}
					reader.readAsText(blob);
				} catch (ex) {
				}
				return false;
			}
		});
		/** @param {number} idx */
		function addResponse(idx)
		{
			if (idx==1 || nmname.val()=='')
				return;
			var mod={'name':nmname.val(),
				'type':'',
				'version':'0.0.1',
				'data':[]};
			var p=nmext.children(':selected').val();
			if (p==-1) //blocks
				mod['type']='block';
			else if (p==-2) //biomes
				mod['type']='biome';
			else if (p==-3) //dimensions
				mod['type']='dimension';
			else
			{
				mod['type']=mods[p]['type'];
				mod['data']=mods[p]['data'].slice();
				mod['version']=mods[p]['version'];
				if (mods[p]['configs']!==undefined)
					mod['configs']=mods[p]['configs'].slice();
			}
			addLocal(mod);
			updatecb();
		}
	}
	/** @param {string} key */
	this.modify=function(key)
	{
		var lastScreen=display.save();
		var mod=locals[key];

		var name=display.toDOM({'input':null,'type':'text','val':mod['name'],'size':'40'});
		var version=display.toDOM({'input':null,'type':'text','val':mod['version'],'size':'8'});
		var cfgstr='';
		if (mod['configs']!=undefined)
			cfgstr=mod['configs'].join(', ');
		var configs=display.toDOM({'input':null,'type':'text','val':cfgstr,'size':'40'});
		var updstr='';
		if (mod['update']!=undefined)
			updstr=mod['update'];
		var update=display.toDOM({'input':null,'type':'text','size':'50','val':updstr});
		var save=display.toDOM({'button':'Save','css':{'float':'right'}});
		save.click(function(){
			var nm=name.val().trim();
			if (nm=='')
			{
				alert("You must give this mod a name");
				return;
			}
			mod['name']=nm;
			mod['version']=version.val().trim();
			var cfgs=configs.val().split(',');
			if (cfgs.length>0)
			{
				mod['configs']=[];
				for (var i=0;i<cfgs.length;i++)
				{
					var c=cfgs[i].trim();
					if (c!='')
						mod['configs'].push(c);
				}
			}
			else
				delete mod['configs'];
			if (mod['configs'].length==0)
				delete mod['configs'];
			var upd=update.val().trim();
			if (upd!='')
				mod['update']=upd;
			else
				delete mod['update'];
			$(window).unbind("scroll");
			display.restore(lastScreen);
			saveLocals();
			updatecb();
		});
		var cancel=display.toDOM({'button':'Cancel','css':{'float':'right'}});
		cancel.click(function(){
			loadLocals();
			$(window).unbind("scroll");
			display.restore(lastScreen);
		});
		var buttons=display.toDOM({'div':[save,cancel,
			{'br':null,'css':{'clear':'right'}}],'class':'buttonbar'});
		var data=display.toDOM({'div':null});
		display.draw([
			{'h1':'Modify Mod'},
			{'table':[
				{'tr':[{'th':'Name:'},{'td':name}]},
				{'tr':[{'th':'Version:'},{'td':version}]},
				{'tr':[{'th':'Configs:'},{'td':configs}]},
				{'tr':[{'th':'Update:'},{'td':update}]}
			]},
			buttons,
			data,
			{'div':null,'css':{'height':'100px'}}
		]);
		name.focus();

		function refresh(){
			var scr=parseInt($(window).scrollTop(),10);
			data.empty();
			switch (mod.type)
			{
				case 'block':
					data.append(display.toDOM(makeBlockData()));
					break;
				case 'biome':
					data.append(display.toDOM(makeBiomeData()));
					break;
				case 'dimension':
					data.append(display.toDOM(makeDimensionData()));
					break;
				default:
					throw "Unknown type: "+mod.type;
			}
			$(window).scrollTop(scr);
		}
		refresh();

		var buttonTop=buttons.offset().top;
		//keep buttons on screen
		$(window).scroll(function() {
			var top=$(window).scrollTop();
			if (top>buttonTop)
				buttons.css({'position':'fixed'});
			else
				buttons.css({'position':'relative'});
		});


		function makeBlockData()
		{
			var tbl=[];
			for (var i=0;i<mod['data'].length;i++)
			{
				var modify=display.toDOM({'button':'Modify'});
				modify.click((function(idx){
					return function() {
						editBlock(idx);
					}})(i));
				var del=display.toDOM({'button':'Delete'});
				del.click((function(idx){
					return function() {
						deleteBlock(idx);
					}})(i));

				var id;
				if (mod['data'][i]['config']!==undefined)
					id=mod['data'][i]['config'];
				else
					id=String(mod['data'][i]['id']);

				tbl.push({'tr':[
					{'td':id},
					{'td':mod['data'][i]['name']},
					{'td':modify},
					{'td':del}
					]});
				if (mod['data'][i]['variants'])
				{
					for (var j=0;j<mod['data'][i]['variants'].length;j++)
					{
						modify=display.toDOM({'button':'Modify'});
						modify.click((function(idx,v){
							return function() {
								editVariant(idx,v);
							}})(i,j));
						del=display.toDOM({'button':'Delete'});
						del.click((function(idx,v){
							return function() {
								deleteVariant(idx,v);
							}})(i,j));
						tbl.push({'tr':[
							{'td':String(mod['data'][i]['variants'][j]['data']),'class':'variant'},
							{'td':mod['data'][i]['variants'][j]['name']},
							{'td':modify},
							{'td':del}
							],'class':'variant'});
					}
				}
				var addvar=display.toDOM({'a':'[Add Variant]','href':'#'});
				addvar.click((function(idx){
					return function() {
						editVariant(idx,-1);
						return false;
					}})(i));
				tbl.push({'tr':[
					{'td':null},
					{'td':addvar}],'class':'variant'});
			}
			var add=display.toDOM({'a':'[Add Block]','href':'#'});
			add.click(function() {
				editBlock(-1);
				return false;
			});
			tbl.push({'tr':[{'td':add}]});
			return {'table':tbl};
		}
		function editBlock(idx)
		{
			var dialog=new Dialog(display,['Save','Cancel'],editResponse);
			var id='';
			if (idx!=-1)
			{
				if (mod['data'][idx]['config']!==undefined)
					id=mod['data'][idx]['config'];
				else
					id=String(mod['data'][idx]['id']);
			}
			var idtxt=display.toDOM({'input':null,'type':'text','val':id,'size':'30'});
			var nmtxt=display.toDOM({'input':null,'type':'text',
				'val':(idx!=-1)?mod['data'][idx]['name']:'','size':'30'});
			var mv=15;
			if (idx!=-1 && mod['data'][idx]['mask']!==undefined)
				mv=mod['data'][idx]['mask'];
			var mask=display.toDOM({'input':null,'type':'text','size':'2',
				'val':String(mv)});
			var curC=0;
			if (idx!=-1 && mod['data'][idx]['color']!==undefined)
				curC=parseInt(mod['data'][idx]['color'],16);
			var color=display.toDOM({'div':null,'class':'color'});
			color.css('background','rgb('+(curC>>16)+','+((curC>>8)&0xff)+','+(curC&0xff)+')');
			color.click(function(){
				pickColor(color,curC,function(c){
					curC=c;
					color.css('background','rgb('+(c>>16)+','+((c>>8)&0xff)+','+(c&0xff)+')');
				});
			});
			var alp=100;
			if (idx!=-1)
			{
				if (mod['data'][idx]['color']===undefined)
					alp=0;
				else if (mod['data'][idx]['alpha']!==undefined)
					alp=Math.round(100*mod['data'][idx]['alpha']);
			}
			var alpha=display.toDOM({'input':null,'type':'text','size':'4',
				'val':String(alp)});
			var flags=[
				display.toDOM({'input':null,'type':'checkbox','val':1}),
				display.toDOM({'input':null,'type':'checkbox','val':2}),
				display.toDOM({'input':null,'type':'checkbox','val':4})
			];
			if (idx!=-1)
				for (var i=0;i<flags.length;i++)
				{
					if (mod['data'][idx]['flags']&flags[i].val())
						flags[i].attr('checked','checked');
				}
			dialog.draw([
				{'dl':[
					{'dt':'ID:'},
					{'dd':idtxt},
					{'dt':'Name:'},
					{'dd':nmtxt},
					{'dt':'Color:'},
					{'dd':color},
					{'dt':'Alpha:'},
					{'dd':alpha},
					{'dt':'Flags:'},
					{'dd':[
						{'label':[flags[0],"Transparent"]},
						{'label':[flags[1],"Transp. Solid"]},
						{'label':[flags[2],"Fluid"]}]},
					{'dt':'Damage Mask:'},
					{'dd':mask}
				]}
			]);
			idtxt.focus();

			function editResponse(btnidx)
			{
				if (btnidx==0) //save
				{
					if (idx==-1)
					{
						idx=mod['data'].length;
						mod['data'][idx]={};
					}
					var id=idtxt.val();
					if ($.isNumeric(idtxt.val()))
					{
						mod['data'][idx]['id']=parseInt(idtxt.val(),10);
						delete mod['data'][idx]['config'];
					}
					else
					{
						mod['data'][idx]['config']=idtxt.val().trim();
						delete mod['data'][idx]['id'];
					}
					mod['data'][idx]['name']=nmtxt.val().trim();
					var al=parseInt(alpha.val(),10)/100;
					if (al!=0.0)
					{
						var cc=curC.toString(16);
						while (cc.length<6)
							cc='0'+cc;
						mod['data'][idx]['color']=cc;
						if (al!=1.0)
							mod['data'][idx]['alpha']=al;
						else
							delete mod['data'][idx]['alpha'];
					}
					else
					{
						delete mod['data'][idx]['color'];
						delete mod['data'][idx]['alpha'];
					}
					var flg=0;
					for (var i=0;i<flags.length;i++)
						if (flags[i].is(":checked"))
							flg|=parseInt(flags[i].val(),10);
					if (flg!=0)
						mod['data'][idx]['flags']=flg;
					else
						delete mod['data'][idx]['flags'];
					var msk=parseInt(mask.val(),10);
					if (msk!=15 && msk!=0)
						mod['data'][idx]['mask']=msk;
					else
						delete mod['data'][idx]['mask'];
					sortMod();
					refresh();
				}
			}
		}
		function deleteBlock(idx)
		{
			if (confirm('Are you sure you wish to delete '+mod['data'][idx]['name']+'?'))
			{
				mod['data'].splice(idx,1);
				refresh();
			}
		}
		function editVariant(idx,v)
		{
			var dialog=new Dialog(display,['Save','Cancel'],editResponse);
			var dtxt=display.toDOM({'input':null,'type':'text',
				'val':(v!=-1)?mod['data'][idx]['variants'][v]['data']:'','size':'4'});
			var nm=mod['data'][idx]['name'];
			var curC=0;
			if (mod['data'][idx]['color']!==undefined)
				curC=parseInt(mod['data'][idx]['color'],16);
			if (v!=-1)
			{
				if (mod['data'][idx]['variants'][v]['name']!==undefined)
					nm=mod['data'][idx]['variants'][v]['name'];
				if (mod['data'][idx]['variants'][v]['color']!==undefined)
					curC=parseInt(mod['data'][idx]['variants'][v]['color'],16);
			}
			var nmtxt=display.toDOM({'input':null,'type':'text','val':nm,'size':'40'});
			var color=display.toDOM({'div':null,'class':'color',
				'css':{'background':'rgb('+(curC>>16)+','+((curC>>8)&0xff)+','+(curC&0xff)+')'}});
			color.click(function(){
				pickColor(color,curC,function(c){
					curC=c;
					color.css('background','rgb('+(c>>16)+','+((c>>8)&0xff)+','+(c&0xff)+')');
				});
			});
			dialog.draw([
				{'dl':[
					{'dt':'Damage:'},
					{'dd':dtxt},
					{'dt':'Name:'},
					{'dd':nmtxt},
					{'dt':'Color:'},
					{'dd':color}
				]}
			]);
			dtxt.focus();
			function editResponse(btnidx)
			{
				if (btnidx==0) //save
				{
					if ($.isNumeric(dtxt.val()))
					{
						if (mod['data'][idx]['variants']===undefined)
							mod['data'][idx]['variants']=[];
						if (v==-1)
						{
							v=mod['data'][idx]['variants'].length;
							mod['data'][idx]['variants'][v]={};
						}
						mod['data'][idx]['variants'][v]['data']=parseInt(dtxt.val(),10);
						var nm=nmtxt.val().trim();
						if (nm!==mod['data'][idx]['name'] && nm!='')
							mod['data'][idx]['variants'][v]['name']=nm;
						else
							delete mod['data'][idx]['variants'][v]['name'];
						var c=curC.toString(16);
						while (c.length<6)
							c='0'+c;
						if (mod['data'][idx]['color']==undefined ||
							c!==mod['data'][idx]['color'])
							mod['data'][idx]['variants'][v]['color']=c;
						else
							delete mod['data'][idx]['variants'][v]['color'];
						sortMod();
						refresh();
					}
				}
			}
		}
		function deleteVariant(idx,v)
		{
			if (confirm("Are you sure you wish to delete this variant?"))
			{
				mod['data'][idx]['variants'].splice(v,1);
				if (mod['data'][idx]['variants'].length==0)
					delete mod['data'][idx]['variants'];
				refresh();
			}
		}
		function makeBiomeData()
		{
			var tbl=[];
			for (var i=0;i<mod['data'].length;i++)
			{
				var modify=display.toDOM({'button':'Modify'});
				modify.click((function(idx){
					return function() {
						editBiome(idx);
					}})(i));
				var del=display.toDOM({'button':'Delete'});
				del.click((function(idx){
					return function() {
						deleteBiome(idx);
					}})(i));
				var id;
				if (mod['data'][i]['config']!==undefined)
					id=mod['data'][i]['config'];
				else
					id=String(mod['data'][i]['id']);
				tbl.push({'tr':[
					{'td':id},
					{'td':mod['data'][i]['name']},
					{'td':modify},
					{'td':del}
					]});
			}
			var add=display.toDOM({'a':'[Add Biome]','href':'#'});
			add.click(function() {
				editBiome(-1);
				return false;
			});
			tbl.push({'tr':[{'td':add}]});
			return {'table':tbl}
		}
		function editBiome(idx)
		{
			var dialog=new Dialog(display,['Save','Cancel'],editResponse);
			var id='';
			if (idx!=-1)
			{
				if (mod['data'][idx]['config']!==undefined)
					id=mod['data'][idx]['config'];
				else
					id=String(mod['data'][idx]['id']);
			}
			var idtxt=display.toDOM({'input':null,'type':'text','val':id,'size':'40'});
			var nmtxt=display.toDOM({'input':null,'type':'text',
				'val':(idx!=-1)?mod['data'][idx]['name']:'','size':'40'});
			dialog.draw([
				{'dl':[
					{'dt':'ID:'},
					{'dd':idtxt},
					{'dt':'Name:'},
					{'dd':nmtxt}
				]}
			]);
			idtxt.focus();

			function editResponse(btnidx)
			{
				if (btnidx==0) //save
				{
					if (idx==-1)
					{
						idx=mod['data'].length;
						mod['data'][idx]={};
					}
					var id=idtxt.val();
					if ($.isNumeric(idtxt.val()))
					{
						mod['data'][idx]['id']=parseInt(idtxt.val(),10);
						delete mod['data'][idx]['config'];
					}
					else
					{
						mod['data'][idx]['config']=idtxt.val().trim();
						delete mod['data'][idx]['id'];
					}
					mod['data'][idx]['name']=nmtxt.val().trim();
					sortMod();
					refresh();
				}
			}
		}
		function deleteBiome(idx)
		{
			if (confirm('Are you sure you wish to delete '+mod['data'][idx]['name']+'?'))
			{
				mod['data'].splice(idx,1);
				refresh();
			}
		}
		function makeDimensionData()
		{
			var tbl=[];
			for (var i=0;i<mod['data'].length;i++)
			{
				var modify=display.toDOM({'button':'Modify'});
				modify.click((function(idx){
					return function() {
						editDimension(idx);
					}})(i));
				var del=display.toDOM({'button':'Delete'});
				del.click((function(idx){
					return function() {
						deleteDimension(idx);
					}})(i));
				tbl.push({'tr':[
					{'td':mod['data'][i]['name']},
					{'td':modify},
					{'td':del}
					]});
			}
			var add=display.toDOM({'a':'[Add Dimension]','href':'#'});
			add.click(function() {
				editDimension(-1);
				return false;
			});
			tbl.push({'tr':[{'td':add}]});
			return {'table':tbl};
		}
		function editDimension(idx)
		{
			var dialog=new Dialog(display,['Save','Cancel'],editResponse);
			var nmtxt=display.toDOM({'input':null,'type':'text',
				'val':(idx!=-1)?mod['data'][idx]['name']:'','size':'40'});
			var pathtxt=display.toDOM({'input':null,'type':'text',
				'val':(idx!=-1)?mod['data'][idx]['path']:'','size':'50'});
			var scaletxt=display.toDOM({'input':null,'type':'text','size':'2',
				'val':(idx!=-1)?String(mod['data'][idx]['scale']):'1'});
			var regex=display.toDOM({'input':null,'type':'checkbox','val':1});
			if (idx!=-1 && mod['data'][idx]['regex']==true)
				regex.attr('checked','checked');
			dialog.draw([
				{'dl':[
					{'dt':'Name:'},
					{'dd':nmtxt},
					{'dt':'Path:'},
					{'dd':pathtxt},
					{'dd':{'label':[regex,"Path is Regex"]}},
					{'dt':'Scale:'},
					{'dd':scaletxt}
				]}
			]);
			nmtxt.focus();

			function editResponse(btnidx)
			{
				if (btnidx==0) //save
				{
					if (idx==-1)
					{
						idx=mod['data'].length;
						mod['data'][idx]={};
					}
					mod['data'][idx]['name']=nmtxt.val().trim();
					mod['data'][idx]['path']=pathtxt.val().trim();
					mod['data'][idx]['scale']=parseInt(scaletxt.val(),10);
					if (regex.is(":checked"))
						mod['data'][idx]['regex']=true;
					else
						delete mod['data'][idx]['regex'];
					refresh();
				}
			}
		}
		function deleteDimension(idx)
		{
			if (confirm('Are you sure you wish to delete '+mod['data'][idx]['name']+'?'))
			{
				mod['data'].splice(idx,1);
				refresh();
			}
		}

		function sortMod()
		{
			mod['data'].sort(function(a,b){
				if (a['id']!==undefined && b['id']!==undefined)
					return a['id']-b['id'];
				if (a['id']!==undefined)
					return -1;
				if (b['id']!==undefined)
					return 1;
				if (a['config']==b['config'])
					return 0;
				return a['config']>b['config']?1:-1;
			});
			for (var i=0;i<mod['data'].length;i++)
			{
				if (mod['data'][i]['variants']!==undefined)
					mod['data'][i]['variants'].sort(function(a,b) {
						return a['data']-b['data'];
					});
			}
		}
	}
	/** @param {string} key */
	this.remove=function(key)
	{
		if (confirm("Are you sure you wish to delete "+locals[key].name+"?"))
		{
			delete locals[key];
			saveLocals();
			updatecb();
		}
	}
	/** @param {string} key */
	this.exportMod=function(key)
	{
		var blob=new Blob([window.JSON.stringify(locals[key])],{'type':'application/octet-stream'});
		var url=window.URL.createObjectURL(blob);
		var savelink=$(document.createElement('a'));
		savelink.attr('href',url);
		savelink.attr('download',key);
		var event=document.createEvent("MouseEvents");
		event.initMouseEvent("click",true,false,window,1,0,0,0,0,
				false,false,false,0,null);
		savelink[0].dispatchEvent(event);
		setTimeout(function(){
				window.URL.revokeObjectURL(url);
		},5000); //5 seconds then nuke url
	}

	this.addPack=function()
	{
		var dialog=new Dialog(display,['Create','Cancel'],addResponse);

		var nmname=display.toDOM({'input':null,'type':'text','size':'30'});
		dialog.draw([
			{'h4':'Add Pack'},
			{'dl':[
				{'dt':'Name:'},
				{'dd':nmname}]}
			]);
		nmname.focus();

		/** @param {number} idx */
		function addResponse(idx)
		{
			if (idx==1 || nmname.val().trim()=='')
				return;
			localPacks.push({'name':nmname.val().trim(),
				'type':'pack',
				'version':'v1',
				'data':[]});
			saveLocalPacks();
			updatecb();
		}
	}
	/** @param {string} key */
	this.modifyPack=function(key)
	{
		var lastScreen=display.save();
		var pack=localPacks[key];

		var name=display.toDOM({'input':null,'type':'text','val':pack['name'],'size':'30'});
		var version=display.toDOM({'input':null,'type':'text','val':pack['version'],'size':'8'});
		var updstr='';
		if (pack['update']!=undefined)
			updstr=pack['update'];
		var update=display.toDOM({'input':null,'type':'text','val':updstr,'size':'50'});

		var save=display.toDOM({'button':'Save','css':{'float':'right'}});
		save.click(function(){
			var nm=name.val().trim();
			if (nm=='')
			{
				alert("You must give this pack a name");
				return;
			}
			pack['name']=nm;
			pack['version']=version.val().trim();
			var upd=update.val().trim();
			if (upd!='')
				pack['update']=upd;
			else
				delete pack['update'];
			pack['data'].splice(0,pack['data'].length);
			for (var i=0;i<allmods.length;i++)
			{
				if (allmods[i].checked)
					pack['data'].push({'type':allmods[i]['type'],'id':allmods[i]['id']});
			}

			$(window).unbind('scroll');
			display.restore(lastScreen);
			saveLocalPacks();
			updatecb();
		});
		var cancel=display.toDOM({'button':'Cancel','css':{'float':'right'}});
		cancel.click(function(){
			loadLocalPacks();
			$(window).unbind('scroll');
			display.restore(lastScreen);
		});
		var buttons=display.toDOM({'div':[save,cancel,
			{'br':null,'css':{'clear':'right'}}],'class':'buttonbar'});
		var data=display.toDOM({'div':null});
		display.draw([
			{'h1':'Modify Pack'},
			{'table':[
				{'tr':[{'th':'Name:'},{'td':name}]},
				{'tr':[{'th':'Version:'},{'td':version}]},
				{'tr':[{'th':'Update:'},{'td':update}]}
			]},
			buttons,
			data,
			{'div':null,'css':{'height':'100px'}}
		]);
		name.focus();
		data.append(display.toDOM(makePackData()));

		var buttonTop=buttons.offset().top;
		//keep buttons on screen
		$(window).scroll(function() {
			var top=$(window).scrollTop();
			if (top>buttonTop)
				buttons.css({'position':'fixed'});
			else
				buttons.css({'position':'relative'});
		});

		var allmods;
		function makePackData()
		{
			var tbl=[];
			allmods=[];
			for (var m in mods)
				allmods.push({'type':'pre','id':m,'checked':false});
			for (var m in locals)
				allmods.push({'type':'local','id':m,'checked':false});
			allmods.sort(function(a,b) {
				if (a['type']==b['type'])
				{
					if (a['id']==b['id'])
						return 0;
					return a['id']>b['id']?1:-1;
				}
				return a['type']=='pre'?1:-1;
			});
			for (var i=0;i<pack['data'].length;i++)
			{
				for (var k=0;k<allmods.length;k++)
				{
					if (pack['data'][i]['type']==allmods[k]['type'] && pack['data'][i]['id']==allmods[k]['id'])
					{
						allmods[k]['checked']=true;
						break;
					}
				}
			}

			for (var i=0;i<allmods.length;i++)
			{
				var check=display.toDOM({'input':null,'type':'checkbox','val':String(i)});
				if (allmods[i].checked)
					check.attr('checked','checked');
				var name;
				if (allmods[i]['type']=='local')
					name=locals[allmods[i]['id']]['name']+' ('+locals[allmods[i]['id']]['type']+')';
				else
					name=mods[allmods[i]['id']]['name']+' ('+mods[allmods[i]['id']]['type']+')';
				var lbl=display.toDOM({'label':[check,name]});
				if (allmods[i].checked)
					lbl.addClass('checked');
				check.change(function(e){
					if ($(e.target).is(":checked"))
					{
						$(e.target.parentNode).addClass('checked');
						allmods[parseInt($(e.target).val(),10)].checked=true;
					}
					else
					{
						$(e.target.parentNode).removeClass('checked');
						allmods[parseInt($(e.target).val(),10)].checked=false;
					}
				});
				tbl.push({'tr':[
					{'td':allmods[i]['type']=='local'?'Your Mod':'Pre-Made'},
					{'td':lbl}
				]});
			}
			return {'table':tbl};
		}
	}
	/** @param {string} key */
	this.removePack=function(key)
	{
		if (confirm("Are you sure you wish to delete "+localPacks[key]['name']+"?"))
		{
			localPacks.splice(key,1); //remove from list
			saveLocalPacks();
			updatecb();
		}
	}
}
