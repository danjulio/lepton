//
// Thermal imaging camera Side 1 (5.5x2.1mm connector)
//

difference() {
  // Bottom plate
  cube([43.5, 62.5, 2]);

  // Hole for barrel jack
  translate([4.5, 13, 1]) {
    cylinder(h=4, d=6.5, center=true, $fn=60);
  }
}