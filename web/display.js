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
 */
function Display()
{
	/** @type {jQuery} */
	var dom=$(document.createElement('div'));
	$(document.body).append(dom);


	var types=['select','button','option','h1','h4','div','br','input','dl','dt','dd','table','tr','td','th','span','a','label'];

	// hides the old screen, creates a new one
	this.save=function()
	{
		var oldDom=dom;
		oldDom.hide();
		dom=$(document.createElement('div'));
		$(document.body).append(dom);
		return oldDom;
	}
	// destroys current screen, unhides old one
	this.restore=function(el)
	{
		dom.remove();
		dom=el;
		dom.show();
	}
	// clears current screen
	this.clear=function()
	{
		dom.empty();
	}

	/**
	 * @param {Object} obj
	 * @returns {jQuery}
	 */
	this.toDOM=function(obj)
	{
		/** @type {string|undefined} */
		var type=undefined;
		for (var i=0;i<types.length;i++)
		{
			if (types[i] in obj)
			{
				type=types[i];
				break;
			}
		}
		if (type===undefined)
		{
			window.console.log(obj);
			throw "Invalid DOM";
		}
		/** @type {jQuery} */
		var el=$(document.createElement(type));
		if (obj[type]!=null)
		{
			if (typeof(obj[type])=='string')
				el.html(obj[type]);
			else if (obj[type] instanceof jQuery)
				el.append(obj[type]);
			else if (Array.isArray(obj[type]))
				for (var i=0;i<obj[type].length;i++)
				{
					if (typeof(obj[type][i])=='string' || obj[type][i] instanceof jQuery)
						el.append(obj[type][i]);
					else
						el.append(this.toDOM(obj[type][i]));
				}
			else
				el.append(this.toDOM(obj[type]));
		}
		for (var key in obj)
		{
			if (key==type) continue;
			if (key=='css')
				el.css(obj[key]);
			else if (key=='class')
				el.addClass(obj[key]);
			else if (key=='val')
				el.val(obj[key]);
			else
				el.attr(key,obj[key]);
		}
		return el;
	}

	/** @param {Object} obj */
	this.draw=function(obj)
	{
		if (obj instanceof jQuery)
			dom.append(obj);
		else if (Array.isArray(obj))
			for (var i=0;i<obj.length;i++)
			{
				if (obj[i] instanceof jQuery)
					dom.append(obj[i]);
				else
					dom.append(this.toDOM(obj[i]));
			}
		else
			dom.append(this.toDOM(obj));
	}
}

/**
 * @constructor
 * @param {Display} display
 * @param {Array.<string>} btns
 * @param {Function} cb
 */
function Dialog(display,btns,cb)
{
	var bg=display.toDOM({'div':null,'class':'dialogbg'});
	display.draw(bg);

	var win=display.toDOM({'div':null,'class':'content'});
	var buttons=[];
	for (var i=0;i<btns.length;i++)
	{
		var b=display.toDOM({'button':btns[i]});
		b.click((function(idx){
			return function() {
				dialog.remove();
				bg.remove();
				cb(idx);
			}
		})(i));
		buttons.push(b);
	}
	buttons.push({'br':null,'css':{'clear':'both'}});
	var dialog=display.toDOM({'div':[win,{'div':buttons,'class':'buttons'}],'class':'dialog'});
	display.draw(dialog);

	dialog.keyup(function(e) {
		if (e.keyCode==13 || e.keyCode==27)
		{
			dialog.unbind("keyup");
			dialog.remove();
			bg.remove();
			cb(e.keyCode==27?1:0);
			return false;
		}
		return true;
	});

	this.close=function()
	{
		dialog.remove();
		bg.remove();
	}

	/** @param {Object} obj */
	this.draw=function(obj)
	{
		if (obj instanceof jQuery)
			win.append(obj);
		else if (Array.isArray(obj))
			for (var i=0;i<obj.length;i++)
			{
				if (obj[i] instanceof jQuery)
					win.append(obj[i]);
				else
					win.append(display.toDOM(obj[i]));
			}
		else
			win.append(display.toDOM(obj));
	}
}
