//
// Thermal imaging camera front (LCD Bezel)
//

difference() {
  // Bezel
  cube([102, 62.5, 2]);

  // LCD cut-out
  translate([24, 4, -1]) {
    cube([62, 49.5, 4]);
  }
}