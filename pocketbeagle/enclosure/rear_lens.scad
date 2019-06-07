//
// Thermal imaging camera "lens" for mounting
// on smooth rear
//
difference() {
  cylinder(h=7, d=35, center=false, $fn=120);

  // Cut-out for Flir Module
  translate([-6, -6.5, -1]) {
    cube([12, 13, 9]);
  }
}