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
 
//HSV color wheel popup

/**
 * @param {Element} src
 * @param {Number} color
 * @param {Function} cb
 */
function pickColor(src,color,cb)
{
	var curR=(color>>16)&0xff;
	var curG=(color>>8)&0xff;
	var curB=color&0xff;
	var wheelSize=200;   //width and height of color wheel
	var wheelThickness=10; //thickness of the color ring
	var wheelGap=2; //gap between wheel and SV square
	var wheelBg=[0,0,0,0]; //transparent black background (rgba)
	
	var diam=wheelSize-wheelThickness*2-wheelGap;
	var side=Math.sqrt(diam*diam/2);

	// all numbers scaled 0-255, returns hsv scaled 0-1
	/**
	 * @param {number} r
	 * @param {number} g
	 * @param {number} b
	 * @returns {Array.<number>}
	 */
	function rgb2hsv(r,g,b)
	{
		var h,s,v;
		var min=Math.min(r,g,b);
		v=Math.max(r,g,b);
		if (v==0) // 0,0,0 = 0,0,0
			return [0,0,0];
		var delta=v-min;
		s=delta/v;
		if (s==0) // grey
			return [0,s,v/255];
		if (r==v) //yellow to magenta
			h=(g-b)/delta;
		else if (g==v) //cyan to yellow
			h=2+(b-r)/delta;
		else	// magenta to cyan
			h=4+(r-g)/delta;
		h/=6;
		if (h<0)
			h+=1;
		return [h,s,v/255];
	}
	// all numbers scaled 0-1.. returns rgb scaled 0-255
	/**
	 * @param {number} h
	 * @param {number} s
	 * @param {number} v
	 * @returns {Array.<number>}
	 */
	function hsv2rgb(h,s,v)
	{
		var r,g,b;
		h*=6;
		var i=Math.floor(h);
		var f=h-i;
		var p=v*(1-s);
		var q=v*(1-f*s);
		var t=v*(1-(1-f)*s);
		switch (i%6)
		{
			case 0: r=v; g=t; b=p; break;
			case 1: r=q; g=v; b=p; break;
			case 2: r=p; g=v; b=t; break;
			case 3: r=p; g=q; b=v; break;
			case 4: r=t; g=p; b=v; break;
			case 5: r=v; g=p; b=q; break;
		}
		return [Math.round(r*255),Math.round(g*255),Math.round(b*255)];
	}
	
	var picker=$(document.createElement('div'));
	picker.addClass('picker');
	var pos=src.offset();
	picker.css('top',pos.top+'px');
	picker.css('left',pos.left+'px');
	$(document.body).append(picker);
	var canvas=$(document.createElement('canvas'));
	canvas.attr('width',wheelSize);
	canvas.attr('height',wheelSize);
	canvas.css('float','left');
	picker.append(canvas);
	var controls=$(document.createElement('div'));
	controls.css('float','left');
	picker.append(controls);
	var colorswatch=$(document.createElement('div'));
	colorswatch.addClass('swatch');
	controls.append(colorswatch);
	var hexcolor=$(document.createElement('input'));
	hexcolor.attr('type','text');
	hexcolor.addClass('hexcolor');
	hexcolor.keyup(keyhc);
	controls.append(hexcolor);
	
	controls.append($(document.createElement('div')).addClass('colorlabel').text('R:'));
	var red=$(document.createElement('input'));
	red.attr('type','text');
	red.addClass('colorinput');
	red.keyup(keyred);
	controls.append(red);
	controls.append($(document.createElement('div')).addClass('colorlabel').text('G:'));
	var green=$(document.createElement('input'));
	green.attr('type','text');
	green.addClass('colorinput');
	green.keyup(keygreen);
	controls.append(green);
	controls.append($(document.createElement('div')).addClass('colorlabel').text('B:'));
	var blue=$(document.createElement('input'));
	blue.attr('type','text');
	blue.addClass('colorinput');
	blue.keyup(keyblue);
	controls.append(blue);
	
	var buttons=$(document.createElement('div'));
	buttons.css('clear','left');
	buttons.css('padding-top','10px');
	controls.append(buttons);
	var okay=$(document.createElement('button'));
	okay.css('float','right');
	okay.text('Okay');
	okay.click(function(){
		picker.remove();
		cb((curR<<16)|(curG<<8)|curB);
	});
	buttons.append(okay);
	var cancel=$(document.createElement('button'));
	cancel.css('float','right');
	cancel.text('Cancel');
	cancel.click(function(){picker.remove();});
	buttons.append(cancel);

	
	var clr=$(document.createElement('br'));
	clr.css('clear','left');
	picker.append(clr);


	hexcolor.focus();
	picker.keyup(function(e) {
		if (e.keyCode==13) { okay.click(); return false; }
		if (e.keyCode==27) { cancel.click(); return false; }
		return true;
	});
	
	canvas.mousedown(function(e){
		var x=e.offsetX;
		var y=e.offsetY;
		var start=Math.round(wheelSize/2-side/2);
		var end=Math.round(wheelSize/2+side/2);
		//inside square?
		if (x>=start && x<end && y>=start && y<end)
			trackSV(x,y);
		else //maybe inside ring then
			trackH(x,y);
		return false;
	});
	
	var ctx=canvas[0].getContext('2d');

	function trackH(x,y)
	{
		var outer=wheelSize/2;
		var inner=wheelSize/2-wheelThickness;
		var dy=y-wheelSize/2;
		var dx=x-wheelSize/2;
		var distance=Math.sqrt(dx*dx+dy*dy);
		if (distance>=inner && distance<outer)
		{
			hsv[0]=1+(Math.atan2(-dy,dx)/(Math.PI*2));
			drawHSV();
			hsvChanged();
			$(canvas).mousemove(function(e){
				dy=e.offsetY-wheelSize/2;
				dx=e.offsetX-wheelSize/2;
				hsv[0]=1+(Math.atan2(-dy,dx)/(Math.PI*2));
				drawHSV();
				hsvChanged();
				return false;
			});
			$(canvas).mouseup(function(){
				$(canvas).unbind('mousemove');
				$(canvas).unbind('mouseup');
				return false;
			});
		}
	}
	
	function trackSV(x,y)
	{
		var start=Math.round(wheelSize/2-side/2);
		var end=Math.round(wheelSize/2+side/2);
		hsv[1]=(x-start)/side;
		hsv[2]=1-((y-start)/side);
		drawHSV();
		hsvChanged();
		$(canvas).mousemove(function(e){
			x=e.offsetX;
			y=e.offsetY;
			if (x<start) x=start;
			if (x>end) x=end;
			if (y<start) y=start;
			if (y>end) y=end;
			hsv[1]=(x-start)/side;
			hsv[2]=1-((y-start)/side);
			drawHSV();
			hsvChanged();
			return false;
		});
		$(canvas).mouseup(function(){
			$(canvas).unbind("mousemove");
			$(canvas).unbind("mouseup");
			return false
		});
	}
	
	function createHueWheel()
	{
		//we just loop through all the pixels, figure out if each pixel
		//is inside the ring or not, and then calculate the hue if it is.
		//This is pretty fast, but only needs to run
		//once, since we'll use it as a backdrop buffer for all
		//future drawing.
		var d=ctx.createImageData(wheelSize,wheelSize);
		var offset=0;
		var outer=wheelSize/2;
		var inner=wheelSize/2-wheelThickness;
		for (var y=0;y<wheelSize;y++)
		{
			var dy=y-wheelSize/2;
			var dy2=dy*dy;
			for (var x=0;x<wheelSize;x++)
			{
				//is pixel inside our color wheel?
				var dx=x-wheelSize/2;
				var distance=Math.sqrt(dx*dx+dy2);
				if (distance>=inner && distance<outer)
				{
					var angle=1+(Math.atan2(-dy,dx)/(Math.PI*2));
					var rgb=hsv2rgb(angle,1,1);
					d.data[offset++]=rgb[0];
					d.data[offset++]=rgb[1];
					d.data[offset++]=rgb[2];
					if (outer-distance<1) //anti alias the outer edge?
						d.data[offset++]=255*(outer-distance);
					else if (distance-inner<1) //anti alias inner edge?
						d.data[offset++]=255*(distance-inner);
					else
						d.data[offset++]=255;
				}
				else
				{
					d.data[offset++]=wheelBg[0];
					d.data[offset++]=wheelBg[1];
					d.data[offset++]=wheelBg[2];
					d.data[offset++]=wheelBg[3];
				}
			}
		}
		return d;
	}
	function drawHandle(d,x,y)
	{
		var circle=	"..111.."+
					".10001."+
					"10...01"+
					"10...01"+
					"10...01"+
					".10001."+
					"..111..";
		var offset=(y-3)*wheelSize*4+(x-3)*4;
		var pos=0;
		for (y=0;y<7;y++,offset+=wheelSize*4-7*4)
			for (x=0;x<7;x++,pos++)
			{
				if (circle[pos]=='.')
					offset+=4;
				else
				{
					var c;
					if (circle[pos]=='1')
						c=0x00;
					else c=0xff;
					d.data[offset++]=c;
					d.data[offset++]=c;
					d.data[offset++]=c;
					d.data[offset++]=0xff;
				}
			}
	}
	function drawHSV()
	{
		ctx.putImageData(colorWheel,0,0);
		var d=ctx.getImageData(0,0,wheelSize,wheelSize);		
		var start=Math.round(wheelSize/2-side/2);
		var end=Math.round(wheelSize/2+side/2);

		for (var y=start;y<end;y++)
		{
			var v=(y-start)/side;
			var offset=y*wheelSize*4+start*4;
			for (var x=start;x<end;x++)
			{
				var s=(x-start)/side;
				var rgb=hsv2rgb(hsv[0],s,1-v);
				d.data[offset++]=rgb[0];
				d.data[offset++]=rgb[1];
				d.data[offset++]=rgb[2];
				d.data[offset++]=0xff;
			}
		}
		
		// now calculate where to draw the selection points
		var angle=hsv[0]*Math.PI*2;
		var h=wheelSize/2-wheelThickness/2;
		var x=Math.round(Math.cos(angle)*h+wheelSize/2);
		var y=Math.round(wheelSize/2-Math.sin(angle)*h);
		drawHandle(d,x,y);
		

		x=Math.round((hsv[1]*side)+start);
		y=Math.round(((1-hsv[2])*side)+start);
		drawHandle(d,x,y);
		ctx.putImageData(d,0,0);
	}
	function hsvChanged()
	{
		var rgb=hsv2rgb(hsv[0],hsv[1],hsv[2]);
		curR=rgb[0];
		curG=rgb[1];
		curB=rgb[2];
		color=(curR<<16)|(curG<<8)|curB;
		updateHex();
		updateRGB();
		updateSwatch();
	}
	function rgbChanged()
	{
		updateSwatch();
		hsv=rgb2hsv(curR,curG,curB);
		drawHSV();
	}
	function updateHex()
	{
		var hexstr=color.toString(16);
		while (hexstr.length<6)
			hexstr='0'+hexstr;
		hexcolor.val(hexstr);
	}
	function updateRGB()
	{
		red.val(String(curR));
		green.val(String(curG));
		blue.val(String(curB));
	}
	function updateSwatch()
	{
		colorswatch.css('background','rgb('+curR+','+curG+','+curB+')');
	}
	function keyhc()
	{
		color=parseInt(hexcolor.val(),16);
		curR=(color>>16)&0xff;
		curG=(color>>8)&0xff;
		curB=color&0xff;
		hsv=rgb2hsv(curR,curG,curB);
		updateRGB();
		updateSwatch();
		drawHSV();
	}
	function keyred()
	{
		curR=parseInt(red.val(),10)&0xff;
		color&=0xffff;
		color|=curR<<16;
		updateHex();
		rgbChanged();
	}
	function keygreen()
	{
		curG=parseInt(green.val(),10)&0xff;
		color&=0xff00ff;
		color|=curG<<8;
		updateHex();
		rgbChanged();
	}
	function keyblue()
	{
		curB=parseInt(blue.val(),10)&0xff;
		color&=0xffff00;
		color|=curB;
		updateHex();
		rgbChanged();
	}
	
	var colorWheel=createHueWheel();
	var hsv=rgb2hsv(curR,curG,curB);
	hsvChanged();
	drawHSV();
}
