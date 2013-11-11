// Text Examples
// By Torsten Paul (t-paul)
// CC-BY-SA license



$fn = 20;
$ff = "DejaVuSansCondensed";
$ff2 = "Amiri-Regular";
 
union() {
#cube([130, 10, 1]);
linear_extrude(height = 2)
text(t = "OpenSCAD with Text!", size = 10, $fn = 40, font = $ff);
}
 
translate([0, -40, 0]) difference() {
cube([90, 14, 1]);
translate([5, 3, -1])
linear_extrude(height = 40)
text(t = "FreeType", size = 10, font = $ff);
}
 
translate([0, -20, 0])
linear_extrude(height = 6)
text(t = "it works !!!", font = $ff, size = 17);
 
s = "OpenSCAD";
translate([45, 60, 5])
rotate([-30, 0, 0])
for (a = [0 : 7])
rotate([0, 0, 135 + 90 / 7 * a])
translate([0, 70, 0])
rotate([90, 0, 180])
translate([-5, 0, 0])
linear_extrude(height = 2)
text(t = s[a], size = 20, font = $ff);
 
translate([0, -80, 0])
rotate([90, 0, 0])
linear_extrude(height = 2)
text(t = "Ленивый рыжий кот", size = 16, font = "DejaVuSansCondensed");
 
translate([0, -120, 0])
rotate([90, 0, 0])
linear_extrude(height = 2)
text(t = "كسول الزنجبيل القط", size = 16, font = $ff2, direction = "rtl", language = "ar", script = "arabic");
 
translate([-20, 0, 0])
linear_extrude(height = 2)
text(t = "懶惰的姜貓", size = 16, font = "AR-PL-New-Sung", direction = "ttb", language = "ch");
 
translate([0, -60, 0])
linear_extrude(height = 2)
text(t = "Umlauts: äöü ÄÖÜ ß", size = 10, font = $ff);
 
translate([0, -80, 0])
linear_extrude(height = 2)
text(t = "Smileys: 😁😂😃😄😅😆😇😈😉😊😋😌😍😎😏😐", size = 10, font = $ff);
 
translate([0, -100, 0])
linear_extrude(height = 2)
text(t = "Braile: ⠁⠂⠃⠄⠅⠆⠇⠈⠉⠊⠋⠌⠍⠎⠏", size = 10, font = $ff);
 
translate([0, -120, 0])
linear_extrude(height = 2)
text(t = "Cards: 🂡🂱🃁🃑", size = 10, font = $ff);
