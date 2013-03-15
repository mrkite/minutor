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

/** @constructor */
function Editor()
{
	/** @type {Display} */
	var display=new Display();
	/** @type {Mods} */
	var mods=new Mods(modschanged,display);
	/** @type {PackGenerator} */
	var generator=new PackGenerator(mods,display);

	/** @type {jQuery} */
	var packs=display.toDOM({'select':null,'size':'6'});
	/** @type {jQuery} */
	var generate=display.toDOM({'button':'Generate','disabled':'true'});
	generate.click(function(){
		generator.generate(packs.children(':selected').val(),false);
	});
	packs.change(function() {
		activate([generate],packs.children(':selected').length>0);
	});

	/** @type {jQuery} */
	var localmods=display.toDOM({'select':null,'size':'6'});
	/** @type {jQuery} */
	var addmod=display.toDOM({'button':'Add Mod'});
	addmod.click(mods.add);
	/** @type {jQuery} */
	var modifymod=display.toDOM({'button':'Modify Mod','disabled':'true'});
	modifymod.click(function(){
		mods.modify(localmods.children(':selected').val());
	});
	/** @type {jQuery} */
	var removemod=display.toDOM({'button':'Remove Mod','disabled':'true'});
	removemod.click(function(){
		mods.remove(localmods.children(':selected').val());
	});
	var exportmod=display.toDOM({'button':'Export Mod','disabled':'true'});
	exportmod.click(function(){
		mods.exportMod(localmods.children(':selected').val());
	});
	localmods.change(function(){
		activate([modifymod,removemod,exportmod],localmods.children(':selected').length>0);
	});

	/** @type {jQuery} */
	var localpacks=display.toDOM({'select':null,'size':'6'});
	/** @type {jQuery} */
	var addpack=display.toDOM({'button':'Add Pack'});
	addpack.click(mods.addPack);
	/** @type {jQuery} */
	var modifypack=display.toDOM({'button':'Modify Pack','disabled':'true'});
	modifypack.click(function(){
		mods.modifyPack(localpacks.children(':selected').val());
	});
	/** @type {jQuery} */
	var generatelocal=display.toDOM({'button':'Generate','disabled':'true'});
	generatelocal.click(function(){
		generator.generate(localpacks.children(':selected').val(),true);
	});
	/** @type {jQuery} */
	var removepack=display.toDOM({'button':'Remove Pack','disabled':'true'});
	removepack.click(function() {
		mods.removePack(localpacks.children(':selected').val());
	});
	localpacks.change(function(){
		activate([modifypack,generatelocal,removepack],localpacks.children(':selected').length>0);
	});

	display.draw([
		{'h1':'Minutor Pack Builder'},
		{'div':[
			{'h4':'Pre-made Packs'},
			packs
			],'css':{'float':'left'}},
		{'div':[
			{'h4':'&nbsp;'},
			generate
			],'css':{'float':'left'}},
		{'div':[
			{'h4':'Your Mods'},
			localmods
			],'css':{'clear':'left','float':'left'}},
		{'div':[
			{'h4':'&nbsp;'},
			addmod,
			modifymod,
			removemod,
			exportmod
			],'css':{'float':'left'}},
		{'div':[
			{'h4':'Your Packs'},
			localpacks
			],'css':{'float':'left'}},
		{'div':[
			{'h4':'&nbsp;'},
			addpack,
			modifypack,
			removepack,
			generatelocal
			],'css':{'float':'left'}}
	]);



	/**
	 * @param {Array.<jQuery>} els
	 * @param {boolean} onoff
	 */
	function activate(els,onoff)
	{
		for (var i=0;i<els.length;i++)
		{
			if (onoff)
				els[i].removeAttr('disabled');
			else
				els[i].attr('disabled','disabled');
		}
	}
	function modschanged()
	{
		/** @type {!jQuery} */
		var item;
		packs.empty();
		for (var pack in mods.packs)
			packs.append(display.toDOM({'option':mods.packs[pack]['name'],'val':pack}));
		localmods.empty();
		for (var mod in mods.locals)
			localmods.append(display.toDOM({'option':mods.locals[mod]['name']+' ('+mods.locals[mod]['type']+')','val':mod}));

		localpacks.empty();
		for (var pack in mods.localPacks)
			localpacks.append(display.toDOM({'option':mods.localPacks[pack]['name'],'val':pack}));
		packs.trigger('change');
		localmods.trigger('change');
		localpacks.trigger('change');
	}
}

/** @type {!Editor} */
var editor;

$(document).ready(function() {
	editor=new Editor();
});
