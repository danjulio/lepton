//
// Thermal imaging camera rear (camera side)
//   Design for external camera "lens" structure
//

difference() {
  // bezel
  cube([102, 62.5, 2]);

  // Cut-out for Flir Module
  translate([45, 29, -1]) {
    cube([12, 13, 4]);
  }

  // Cut-out for power button
  translate([83.5, 10, 1]) {
    cylinder(h=4, d=6, center=true, $fn=60);
  }
}