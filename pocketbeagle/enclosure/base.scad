// Thermal imaging camera bottom plate
//
// View from front - (0, 0, 0) = front, left, bottom
//

//
// Main parts of plate
//
module base_parts() {
  union() {
    // Bottom plate
    cube([98, 43.1, 2]);

    //
    // Pi Platter section -> front
    //
    translate([0, 0, 0]) {
      cube([9.75, 14.6, 12]);  // Left mount
    }
    translate([22, 9, 0]) {
      cube([10, 5.6, 8]);      // Left PCB base
    }
    translate([56, 9, 0]) {
      cube([42, 5.6, 8]);      // Right PCB base
    }
    translate([12, 12.6, 0]) {
      cube([10, 2, 13]);       // Left PCB guide (rear of PCB)
    }
    translate([22, 9, 0]) {
      cube([10, 2, 13]);       // Middle Left PCB guide (front of PCB)
    }
    translate([59, 9, 0]) {
      cube([8, 2, 13]);        // Middle Right PCB guide (front of PCB)
    }
    translate([79, 12.6, 0]) {
      cube([10, 2, 13]);       // Right PCB guide (rear of PCB)
    }
    translate([88, 0, 0]) {
      cube([10, 5.5, 12]);     // Right mount
    }
    translate([88, 0, 0]) {
      cube([10, 9, 8]);        // Right mount extension
    }

    //
    // Battery section
    //
    translate([56, 14.6, 0]) {
      cube([42, 9.5, 6]);      // Battery base
    }

    //
    // Pocketbeagle section -> rear
    //
    translate([0, 27.1, 0]) {
      cube([15, 10, 15.5]);      // Left mount
    }
    translate([0, 24.1, 0]) {
      cube([20, 5, 8]);          // Left PCB base
    }
    translate([0, 37.1, 0]) {
      cube([5.5, 6, 15.5]);      // Left front mount
    }
    translate([56, 24.1, 0]) {
      cube([42, 9, 8]);          // Right PCB base
    }
    translate([67, 27.1, 0]) {
      cube([8, 2, 12]);          // Right PCB guide (front of PCB)
    }
    translate([80, 27.1, 0]) {
      cube([18, 10, 14.5]);      // Right mount
    }
    translate([92.5, 37.1, 0]) {
      cube([5.5, 6, 14.5]);
    }
  }
}

//
// Plate with subtraction for tripod-mount
//
difference() {
  base_parts();

  translate([70, 18.25, -1]) {
    cylinder(h=20, d=13.25, center=false, $fn=6);
  }
}