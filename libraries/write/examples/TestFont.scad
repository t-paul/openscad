use <write/write.scad>

//New
writesphere("Be kind to others",[0,0,18],40.5,font="Braille",space=1.2);

//Original (DXF)
writesphere("Be kind to others",[0,0,12],40.5,font="braille.dxf",space=1.2);
writesphere("Be kind to others",[0,0,6],40.5,space=1.2); //This will find the default font  as letters.dxf and will thus go to the old implementation.


writesphere("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 ;:.,!?"
                     ,[0,0,0],40.5,font="orbitron.dxf",space=1.1);


//Original
writesphere("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 ;:.,!?"
		,[0,0,-6],40.5,font="braille.dxf",space=1.1);

//New
writesphere("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 ;:.,!?"
		,[0,0,-12],40.5,font="Braille",space=1.1);

writesphere("Ленивый рыжий кот"
		,[0,0,-18],40.5,font="DejaVuSansCondensed",space=1.1);

writesphere("كسول الزنجبيل القط"
		,[0,0,-24],40.5,font = "Amiri-Regular", direction = "rtl", language = "ar", script = "arabic",space=1.1);

writesphere("懶惰的姜貓"
		,[0,0,-30],40.5,font = "AR-PL-New-Sung", direction = "ttb", language = "ch",space=1.1);


translate([0, -80, 0])
rotate([90, 0, 0])
linear_extrude(height = 2)
text(t = "Ленивый рыжий кот", size = 16, font = "DejaVuSansCondensed");