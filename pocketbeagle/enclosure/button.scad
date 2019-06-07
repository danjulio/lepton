//
// Thermal imaging camera power button
//
module additive_parts() {
  // Disk
  cylinder(h=5.5, d=10, center=false, $fn=60);

  // Stem
  cylinder(h=10, d=5, center=false, $fn=60);
}

difference() {
  additive_parts();

  // Cut-out for PCB button stem
  cylinder(h=2, d=4.5, center=false, $fn=30);
}