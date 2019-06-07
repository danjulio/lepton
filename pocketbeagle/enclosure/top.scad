// Thermal imaging camera top plate
//
// View from bottom left corner
//

//
// Main parts of plate
//
module additive_parts() {
  union() {
    // Top plate
    cube([98, 43.1, 2]);

    //
    // Pi Platter section -> front
    //
    translate([0, 0, 0]) {
      cube([10, 7, 8]);          // Left mount
    }
    translate([31, 0, 0]) {
      cube([9, 11, 10]);         // Middle mount
    }
    translate([31, 9, 0]) {
      cube([9, 5.6, 4]);         // Middle PCB base
    }
    translate([88.25 , 0, 0]) {
      cube([9.75, 14.6, 10]);    // Right mount
    }
    translate([81, 9, 0]) {
      cube([9, 5.6, 4]);        // Right PCB base
    }

    //
    // Pocketbeagle section -> rear
    //
    translate([0, 27.1, 0]) {
      cube([18, 10, 10.5]);      // Left mount
    }
    translate([0, 37.1, 0]) {
      cube([5.5, 6, 10.5]);      // Left PCB retainer
    }
    translate([68, 24.1, 0]) {
      cube([30, 3, 4]);          // Right PCB base
    }
    translate([92.5, 34.1, 0]) {
      cube([5.5, 9, 10.5]);      // Right mount
    }
  }
}

//
// Plate with subtractions for holes
//
difference() {
  additive_parts();

  //
  // USB 1
  //
  translate([42.5, 3.25, -1]) {
    cube([15.25, 7.5, 4]);
  }

  //
  // USB 2
  //
  translate([64.5, 3.25, -1]) {
    cube([15.25, 7.5, 4]);
  }

  //
  // LED
  //
  translate([39, 30, 1]) {
    cylinder(h=4, d=5.5, center=true, $fn=60);
  }
}