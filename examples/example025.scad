// Text Examples -- Simple Font
// By Torsten Paul (t-paul) and Brody Kenrick (brodykenrick)
// CC-BY-SA license

enable_showcase = false; //Enable this to see more of the examples -- they are just slow on the first render

$fn = 20;
$ff = "DejaVuSansCondensed";
//TODO:Decide which font is going to be used in this example
// -- needs to be on all test systems and preferably on all user systems

//Rotate a word
//Iterate across individual letters (chars) 
s = "OpenSCAD";
translate([90, 60, 5])
rotate([-30, 0, 0])
for (a = [0 : (len(s)-1)])
  rotate([0, 0, 135 + (90 / 7) * a])
  translate([0, 100, 0])
  rotate([90, 0, 180])
  translate([-5, 0, 0])
  linear_extrude(height = 4)
  text(t = s[a], size = 20, font = $ff);

if(enable_showcase)
{

translate([0, -20, 0]) 
union() {
color("red")
cube([165, 10+5, 1]);
color("yellow")
linear_extrude(height = 2)
translate([0, +2.5, 0]) 
text(t = "OpenSCAD now has text()!", size = 10, $fn = 40, font = $ff);
}
 
translate([0, -40, 0])
difference() {
  cube([90, 14, 1]);
  translate([5, 3, -1])
  linear_extrude(height = 40)
  text(t = "FreeType", size = 10, font = $ff);
}
 
translate([0, -60, 0])
linear_extrude(height = 2)
text(t = "Umlauts: äöü ÄÖÜ ß", size = 10, font = $ff);
 
translate([0, -80, 0])
linear_extrude(height = 2)
text(t = "Smileys: 😁😂😃😄😅😆😇😈😉😊😋😌😍😎😏😐", size = 8, font = $ff);
 
translate([0, -100, 0])
linear_extrude(height = 2)
text(t = "Braile: ⠁⠂⠃⠄⠅⠆⠇⠈⠉⠊⠋⠌⠍⠎⠏", size = 10, font = $ff);
 
translate([0, -120, 0])
linear_extrude(height = 2)
text(t = "Cyrillic: Ленивый рыжий кот", size = 10, font = $ff);

translate([0, -160, 0])
linear_extrude(height = 2)
text(t = "Cards: 🂡🂱🃁🃑", size = 20, font = $ff);

}
