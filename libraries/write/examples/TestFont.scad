use <write/write.scad>


//New
writesphere("Be kind to others",[0,0,-18],40.5,font="Braille",space=1.2);
writesphere("Be kind to others",[0,0,-24],40.5,font="Inconsolata",space=1.2); //Need to add a font to not go to default old-style font

writesphere("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 ;:.,!?",[0,0,-30],40.5,font="Braille",space=1.1);


writesphere( "Ленивый  рыжий  кот",unicode=true,[0,0,-36],40.5,font="DejaVuSansCondensed",space=1.1);

//Use double spaces in unicode (the implementation handling unicode chars isn't very smart)
writesphere("كسول  الزنجبيل  القط",unicode=true,[0,0,-42],40.5,font = "Amiri-Regular", direction = "rtl", language = "ar", script = "arabic",space=1.1);


//Original (DXF)
writesphere("Be kind to others",[0,0,12],40.5,font="braille.dxf",space=1.2);
writesphere("Be kind to others",[0,0,6],40.5,space=1.2);
writesphere("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 ;:.,!?"
                     ,[0,0,0],40.5,font="orbitron.dxf",space=1.1);
writesphere("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 ;:.,!?"
		,[0,0,-6],40.5,font="braille.dxf",space=1.1);




