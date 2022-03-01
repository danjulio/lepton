//
// tCam-Mini Rev 4 PCB
//

module esp32() {
    // PCB Base
    cube([18, 31.4, 0.8]);
    
    // Metal can
    translate([1.075, 0.5, 0.8]) {
        cube([15.85, 24, 2.5]);
    }
}


module tCamMini() {
    difference() {
        union() {
            // PCB
            cube([38.1, 47, 1.6]);
        
            // ESP32
            translate([10 + 18, 15.3, 0]) {
                rotate([0, 180, 0]) {
                    esp32();
                }
            }
            
            // Parts on top
            translate([0, 0, 1.6]) {
                // USB Connector
                translate([8.2, 0, 0]) {
                    cube([9, 7.4, 3.35]);
                }
                
                // LED
                translate([21.36, 1.915, 0]) {
                    cube([3, 1.25, 1]);
                }
                
                // Button
                translate([26.86, 0.69, 0]) {
                    cube([4.7, 3.7, 2]);
                }
                // Button stem
                translate([27.96, -0.3, 1]) {
                    cube([2.5, 1.4, 1]);
                }
                
                // Lepton
                translate([19.05 - 12.6/2, 25.4 - 11.5/2, 0]) {
                    cube([12.6, 11.5, 7]);
                }
            }
        }
        
        // Mounting holes
        translate([2.54, 2.54, -1]) {
            cylinder(h=3.6, r=1.6, $fn=120);
        }
        
        translate([35.56, 2.54, -1]) {
            cylinder(h=3.6, r=1.6, $fn=120);
        }
        
        translate([2.54, 44.46, -1]) {
            cylinder(h=3.6, r=1.6, $fn=120);
        }
        
        translate([35.56, 44.46, -1]) {
            cylinder(h=3.6, r=1.6, $fn=120);
        }
    }
}

tCamMini();