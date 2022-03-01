//
// tCam-Mini Rev 2 or 4 PCB basic enclosure for printing on a
// FDM 3D printer.
//
// Copyright 2022 (c) Dan Julio
//
// Apologies as I'm not a mechanical engineer and this is a hack...  But it
// got the job done.
//
// Dimensions are mm.
//
// Version 1.0 - Initial release
//
// tcam_mini_basic_encl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// tcam_mini_basic_encl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with tcam_mini_basic_encl.  If not, see <https://www.gnu.org/licenses/>.
//
use <../openscad_libraries/smooth_prim.scad>
use <../openscad_libraries/tcam_mini.scad>

//
// tCam-Mini revision selection
//   1 - Original Rev 2 board (Through-hole LED)
//   2 - Rev 4 board (SMD LED)
//
revision = 2;

//
// Render control
//   1 - Draw Base for STL export
//   2 - Draw Bezel for STL export
//   3 - Draw all parts (including tCam-Mini) for debug
//
render_mode = 3;


//
// Definitions
//

// tCam-Mini PCB dimensions (taken from tcam_mini)
pcb_width = 38.1;
pcb_length = 47;
pcb_height = 1.6;
assy_btm_height = 2.5;
assy_top_height = 6;

// Screw holes (from PCB corner, mirrored 4 places)
hole_x = 2.54;
hole_y = 2.54;

// tCam-Mini USB (from top-left corner of PCB - min x/y)
usb_x = (revision == 1) ? 7.4 : 8.2;
usb_y = (revision == 1) ? 0 : 0;
usb_z = (revision == 1) ? pcb_height - 0.3 : pcb_height;
usb_w = (revision == 1) ? 8.2 : 9;
usb_h = (revision == 1) ? 3.5 : 3.5;

// tCam-Mini LED (from top-left corner of PCB - center x/y)
// Declare variables here, configured in rev_setup()
led_x = (revision == 1) ? 21.59 : 22.86;
led_y = (revision == 1) ? 0 : 0;
led_z = (revision == 1) ? pcb_height + 1.5 : pcb_height + 1;
led_d = (revision == 1) ? 3.5 : 2;

// tCam-Mini Button (from top-left corner of PCB - center x/y)
btn_x = 29.21;
btn_y = 0;
btn_z = pcb_height + 1.5;
btn_d = 1.5;

// ESP32 module
esp32_w = 15.85;
esp32_l = 31.4;
esp32_can_d = 24;
esp32_x = (pcb_width - esp32_w) / 2;
esp32_y = (pcb_length - esp32_l - 0.5) + 0.75;
esp32_tol = 0.5;

// Lepton assy (from top-left corner of PCB - min x/y)
lep_w = 13;
lep_l = 12;
lep_x = (pcb_width - lep_w) / 2;
lep_y = (25.4 - lep_l/2);
lep_z_wall = 0.5;

// Lepton lens hole (from top-left corner of PCB - center x/y)
lep_hole_x = 19.05;
lep_hole_y = 25.4;
lep_hole_d = 5;

// Enclosure base dimensions
pcb_tolerance = 0.3; // Distance between enclosure wall and PCB
wall_thickness = 2;
encl_width = pcb_width + 2 * (pcb_tolerance + wall_thickness);
encl_length = pcb_length + 2 * (pcb_tolerance + wall_thickness);
encl_height = 2*wall_thickness + assy_btm_height + pcb_height + assy_top_height;

// Enclosure bezel dimensions
bez_tolerance = 0.2; // Distance between enclosure wall and bezel
bez_width = encl_width - 2 * (wall_thickness + bez_tolerance);
bez_length = encl_length - 2 * (wall_thickness + bez_tolerance);

// Offsets to corner of bezel in Base
encl_x_bez_offset = wall_thickness + bez_tolerance;
encl_y_bez_offset = wall_thickness + bez_tolerance;
encl_z_bez_offset = encl_height;

// Offsets to corner of PCB Assy
encl_x_pcb_offset = wall_thickness + pcb_tolerance; 
encl_y_pcb_offset = wall_thickness + pcb_tolerance;
encl_z_pcb_offset = wall_thickness + assy_btm_height;

bez_x_pcb_offset = pcb_tolerance - bez_tolerance;
bez_y_pcb_offset = pcb_tolerance - bez_tolerance;

// Stand-offs
screw_d = 2.5;
screw_head_d = 6;

base_standoff_w = 2 * (hole_x + pcb_tolerance);
base_standoff_l = 2 * (hole_y + pcb_tolerance);
base_standoff_c_x = [
    encl_x_pcb_offset + hole_x,
    encl_x_pcb_offset + pcb_width - hole_x,
    encl_x_pcb_offset + hole_x,
    encl_x_pcb_offset + pcb_width - hole_x];
base_standoff_c_y = [
    encl_y_pcb_offset + hole_y,
    encl_y_pcb_offset + hole_y,
    encl_y_pcb_offset + pcb_length - hole_y,
    encl_y_pcb_offset + pcb_length - hole_y];
    
bez_standoff_w = 2 * hole_x;
bez_standoff_l = 2 * hole_y;
bez_standoff_c_x = [
    bez_x_pcb_offset + hole_x, 
    bez_x_pcb_offset + pcb_width - hole_x, 
    bez_x_pcb_offset + hole_x, 
    bez_x_pcb_offset + pcb_width - hole_x];
bez_standoff_c_y = [
    bez_y_pcb_offset + hole_y, 
    bez_y_pcb_offset + hole_y,
    bez_y_pcb_offset + pcb_length - hole_y, 
    bez_y_pcb_offset + pcb_length - hole_y];


// Text
lbl_x_offset = 5;
lbl_y_offset = 4.5;
lbl_size = 6;


//
// Modules
//
module base_standoff(x, y) {
    // Stand-off
    translate([x - base_standoff_w/2, y - base_standoff_l/2, 0]) {
        cube([base_standoff_w, base_standoff_l, wall_thickness + assy_btm_height]);
    }
}


module base_standoff_screw_holes(x, y) {
    // Screw hole
    translate([x, y, -0.1]) {
        cylinder(h = wall_thickness + assy_btm_height + 0.2, r = screw_d/2, $fn=120);
    }
    translate([x, y, -0.1]) {
        cylinder(h = wall_thickness, r1 = screw_head_d/2, r2 = screw_d/2, $fn=120);
    }
}


module bezel_standoff(x, y) {
    // Stand-off
    translate([x - bez_standoff_w/2, y - bez_standoff_l/2, 0]) {
        cube([bez_standoff_w, bez_standoff_l, wall_thickness + assy_top_height]);
    }
}


module bezel_standoff_screw_hole(x, y) {
    // Screw hole
    translate([x, y, wall_thickness]) {
        cylinder(h = assy_top_height + 0.2, r = screw_d/2, $fn=120);
    }
}


module base_assy() {
    difference() {
        union() {
            // Base box
            SmoothHollowCube([encl_width, encl_length, encl_height], wall_thickness, 0);
            
            // Floor
            translate([wall_thickness/2, wall_thickness/2, 0]) {
                cube([encl_width -  wall_thickness, encl_length - wall_thickness, wall_thickness]);
            }
            
            // Stand-offs
            for (i = [0:3]) {
                base_standoff(base_standoff_c_x[i], base_standoff_c_y[i]);
            }
        }
        
        // Screw holes
        for (i = [0:3]) {
                base_standoff_screw_holes(base_standoff_c_x[i], base_standoff_c_y[i]);
        }
        
        // USB Cutout
        translate([encl_x_pcb_offset + usb_x, usb_y - 0.1, encl_z_pcb_offset + usb_z]) {
            cube([usb_w, wall_thickness + 0.2, usb_h]);
        }
        translate([encl_x_pcb_offset + usb_x - 1, usb_y - 0.1, encl_z_pcb_offset + usb_z - 1]) {
            cube([usb_w + 2, 1.2, usb_h + 2]);
        }
        
        // LED Cutout
        translate([encl_x_pcb_offset + led_x, led_y - 0.1, encl_z_pcb_offset + led_z]) {
            rotate([-90, 0, 0]) {
                cylinder(h=wall_thickness + 0.2, r=led_d/2, $fn=120);
            }
        }
        
        // Button Cutout
        translate([encl_x_pcb_offset + btn_x, btn_y - 0.1, encl_z_pcb_offset + btn_z]) {
            rotate([-90, 0, 0]) {
                cylinder(h=wall_thickness + 0.2, r=btn_d/2, $fn=120);
            }
        }
        
        // ESP32 Module Cutout
        translate([encl_x_pcb_offset + esp32_x - esp32_tol, encl_x_pcb_offset + esp32_y - esp32_tol, -0.1]) {
            cube([esp32_w + 2*esp32_tol, esp32_can_d + 2*esp32_tol, wall_thickness + 0.2]);
        }
    }
}


module top_assy() {
    difference() {
        union() {
            // Front bezel
            cube([bez_width, bez_length, wall_thickness]);
            
            // Stand-offs
            for (i = [0:3]) {
                    bezel_standoff(bez_standoff_c_x[i], bez_standoff_c_y[i]);
            }
        }
        
        // Screw holes in stand-offs
        for (i = [0:3]) {
            bezel_standoff_screw_hole(bez_standoff_c_x[i], bez_standoff_c_y[i]);
        }
        
        // Lepton module Cutout
        translate([bez_x_pcb_offset + lep_x, bez_y_pcb_offset + lep_y, lep_z_wall]) {
            cube([lep_w, lep_l, wall_thickness + 0.2]);
        }
        
        // Lepton lens hole
        translate([bez_x_pcb_offset + lep_hole_x, bez_y_pcb_offset + lep_hole_y, -0.1]) {
            cylinder(h=wall_thickness + 0.2, r=lep_hole_d/2, $fn=120);
        }
        
        // Text
        translate([lbl_x_offset, lbl_y_offset, 0.5]) {
            rotate([180, 0, 90]) {
                linear_extrude(0.6) {
                    text("tCam-Mini", font="Helvetica", size=lbl_size);
                }
            }
        }
    }
}


//
// Render Code
//
if (render_mode == 1) {
    base_assy();
}

if (render_mode == 2) {
    top_assy();
}

if (render_mode == 3) {
    #base_assy();
    
    translate([encl_x_bez_offset, encl_y_bez_offset, encl_z_bez_offset]) {
        rotate([0, 180, 0]) {
            translate([-bez_width, 0, 0]) {
                #top_assy();
            }
        }
    }
    
    translate([encl_x_pcb_offset, encl_y_pcb_offset, encl_z_pcb_offset]) {
        tCamMini();
    }
}