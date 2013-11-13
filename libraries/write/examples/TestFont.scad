use <write/write.scad>
write_lib_version = 4.0;
//use <write_original_v3/write.scad>
//write_lib_version = 3.0;

echo ("Using write.scad library version = ", write_lib_version);
z_offset = 6;

//Original (DXF)
echo ("Original (dxf-based) usage of write.scad");
color("red")
writesphere("Write + .dxf import",[0,0,3*z_offset],40.5,font="Letters.dxf",space=1.2);

writesphere("Be kind to others",[0,0,2*z_offset],40.5,font="braille.dxf",space=1.2);

writesphere("Be kind to others",[0,0,1*z_offset],40.5,space=1.2);
writesphere("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 ;:.,!?"
                     ,[0,0,0],40.5,font="orbitron.dxf",space=1.1);
writesphere("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 ;:.,!?"
		,[0,0,-1*z_offset],40.5,font="braille.dxf",space=1.1);




//New
if(write_lib_version >= 4.0)
{
echo ("New (font-based) usage of write.scad");
color("gold")
writesphere("Write + text() - もしもし",[0,0,-3*z_offset],40.5,font="HanaMinA",space=1.2);



//"Be kind to others"
writesphere("⠠⠃⠑⠀⠅⠊⠝⠙⠀⠞⠕⠀⠕⠞⠓⠑⠗⠎",[0,0,-4*z_offset],40.5,font="DejaVuSansCondensed",space=1.2);
writesphere("Be kind to others",[0,0,-5*z_offset],40.5,font="Inconsolata",space=1.2); //Need to add a font to not go to default old-style font

//"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890"
writesphere("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 ;:.,!?"
                     ,[0,0,-6*z_offset],40.5,font="OrbitronBold",space=1.1);
writesphere("⠁⠃⠉⠙⠑⠋⠛⠓⠊⠚⠅⠇⠍⠝⠕⠏⠟⠗⠎⠞⠥⠧⠺⠭⠽⠵⠠⠁⠠⠃⠠⠉⠠⠙⠠⠑⠠⠋⠠⠛⠠⠓⠠⠊⠠⠚⠠⠅⠠⠇⠠⠍⠠⠝⠠⠕⠠⠏⠠⠟⠠⠗⠠⠎⠠⠞⠠⠥⠠⠧⠠⠺⠠⠭⠠⠽⠠⠵⠀⠼⠁⠃⠉⠙⠑⠋⠛⠓⠊⠚",[0,0,-7*z_offset],40.5,font="DejaVuSansCondensed",space=1.1);


writesphere( "Ленивый рыжий кот",unicode=true,[0,0,-8*z_offset],40.5,font="DejaVuSansCondensed",space=1.2);

//Use double spaces in unicode (the implementation handling unicode chars isn't very smart)
writesphere("كسول الزنجبيل القط",unicode=true,[0,0,-9*z_offset],40.5,font = "Amiri-Regular", direction = "rtl", language = "ar", script = "arabic",space=1.1);

writesphere("😁😂😃😄😅😆😇😈😉😊😋😌😍😎😏😐",[0,0,-10*z_offset],40.5,font="DejaVuSansCondensed",space=1.5);

writesphere("🂡🂱🃁🃑",[0,0,-11*z_offset],40.5,font="DejaVuSansCondensed",space=1.8);

}
