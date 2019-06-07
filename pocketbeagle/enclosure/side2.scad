//
// Thermal imaging camera Side 2 (Micro-USB connector)
//

difference() {
  // Bottom plate
  cube([43.5, 62.5, 2]);

  // Cutout for USB connector
  translate([9, 25, -1]) {
    cube([3.5, 8.5, 4]);
  }
}