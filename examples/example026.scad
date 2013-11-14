// Text Examples -- Other Fonts
// By Torsten Paul (t-paul) and Brody Kenrick (brodykenrick)
// CC-BY-SA license

enable_showcase = false; //Enable this to see more of the examples -- they are just slow on the first render

$fn = 20;

//TODO:Decide which fonts are going to be used in this example
// -- needs to be on all test systems and preferably on all user systems
japanese_font = "HanaMinA";
chinese_font  = "AR PL UKai CN";
//chinese_font  = "AR PL UMing CN";
arabic_font   = "Amiri-Regular";



//Rotate a word (in Japanese)
//Iterate across individual letters (unicode) 
s_u = "もしもし";
rotate([-30, 0, 0])
for (a = [0 : (len(s_u)-1)])
  rotate([0, 0, 135 + 90 / (len(s_u)-1) * a])
  translate([0, 70, 0])
  rotate([90, 0, 180])
  translate([-5, 0, 0])
  linear_extrude(height = 2)
  text(t = s_u[a], size = 15, font = japanese_font, language="jp");
 
if(enable_showcase)
{
 
translate([0, -20, 0])
rotate([90, 0, 0])
linear_extrude(height = 2)
text(t = "كسول الزنجبيل القط", size = 16, font = arabic_font, direction = "rtl", language = "ar", script = "arabic");

rotate([90, 0, 0]) 
translate([-20, -40, -100])
linear_extrude(height = 2)
text(t = "懶惰的姜貓", size = 16, font = chinese_font, direction = "ttb", language = "ch");

}
 
