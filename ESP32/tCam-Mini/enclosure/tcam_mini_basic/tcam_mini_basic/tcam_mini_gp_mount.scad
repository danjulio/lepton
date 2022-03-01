//
// GoPro Compatible mount for tCam-Mini enclosure.  Designed to be
// printed on a FDM 3D printer.
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
// tcam_mini_gp_mount is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// tcam_mini_gp_mount is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with tcam_mini_gp_mount.  If not, see <https://www.gnu.org/licenses/>.
//

//
// Render control
//   1. Draw vertical mount piece for STL export (2X used)
//   2. Draw horizontal mount piece for STL export (1X used)
//   3. Draw complete assembly for debug
//
render_mode = 3;

//
// Definitions
//

// tCam-Mini enclosure dimensions
tcam_mini_width = 42.4;
tcam_mini_length = 51.5;
tcam_mini_height = 14;

// Tolerance - added to tcam_mini dimensions to account for print slop
tolerance = 0.2;

// Wall thickness - vertical walls and overhangs
wall_thickness = 2;

// Horizontal (stabilizing) piece
hor_width = 10;
hor_cutout_width = 2.5;
hor_cutout_distance = 6;
hor_length = tcam_mini_length + tolerance + 2 * wall_thickness;
hor_span_height = 1;
hor_wall_height = 4.5;

// Vertical (dual pieces with GoPro mounting holes)
ver_span_height = 3;
ver_mount_offset = 6.5;  // Additional distance to GoPro mount hole
ver_height = ver_span_height + tcam_mini_height + wall_thickness + tolerance;
ver_body_length = tcam_mini_width + 2 * wall_thickness + tolerance;
ver_width = 2.75;
ver_overhang_length = 2.5;
ver_mount_d = 15;  // Diameter of GoPro mounting circular component
ver_hole_d = 5.5;  // Diameter of GoPro mounting hole
ver_h_cutout_width = hor_width - 2*hor_cutout_width;


//
// Modules
//
// vertical mount is oriented on its side for printing
module vertical_mount() {
    difference() {
        union() {
            // Rectangular body part
            cube([ver_height, ver_body_length, ver_width]);
            
            // Rectangular mount part
            translate([(ver_height - ver_mount_d)/2, ver_body_length, 0]) {
                cube([ver_mount_d, ver_mount_offset, ver_width]);
            }
            
            // Circular mount part
            translate([ver_height/2, ver_body_length + ver_mount_offset, 0]) {
                cylinder(h = ver_width, r = ver_mount_d/2, $fn=120);
            }
            
            // Added support cylinders to mate body & mount parts more securely
            translate([(ver_height - ver_mount_d)/2, ver_body_length, 0]) {
                cylinder(h = ver_width, r = (ver_height - ver_mount_d)/2, $fn=120);
            }
            translate([(ver_height - ver_mount_d)/2 + ver_mount_d, ver_body_length, 0]) {
                cylinder(h = ver_width, r = (ver_height - ver_mount_d)/2, $fn=120);
            }
        }
        
        // Cut-outs
        
        // Camera body cut-out
        translate([wall_thickness, wall_thickness, -0.1]) {
            cube([tcam_mini_height + tolerance, tcam_mini_width + tolerance, ver_width + 0.2]);
        }
        
        // Camera front-bezel cut-out
        translate([-0.1, wall_thickness + ver_overhang_length, -0.1]) {
            cube([wall_thickness+0.2, tcam_mini_width - 2*ver_overhang_length, ver_width + 0.2]);
        }
        
        // Horizontal alignment cut-out
        translate([wall_thickness + tcam_mini_height + tolerance/2 - 0.1, wall_thickness + tcam_mini_width/2 + tolerance/2 - ver_h_cutout_width/2, -0.1]) {
            cube([hor_span_height + 0.1, ver_h_cutout_width, ver_width + 0.2]);
        }
        
        // GoPro mounting hole cut-out
        translate([ver_height/2, ver_body_length + ver_mount_offset, -0.1]) {
            cylinder(h = ver_width + 0.2, r = ver_hole_d/2, $fn=120);
        }
    }
}


module horizontal_mount() {
    difference() {
        union() {
            // Span
            cube([hor_width, hor_length, hor_span_height]);
            
            // Vertical walls
            translate([0, 0, 0]) {
                cube([hor_width, wall_thickness, hor_span_height + hor_wall_height]);
            }
            translate([0, hor_length - wall_thickness, 0]) {
                cube([hor_width, wall_thickness, hor_span_height + hor_wall_height]);
            }
        }
        
        // Cut-outs
        translate([
            -0.1, 
            hor_length/2 - hor_cutout_distance/2 - ver_width/2 - tolerance/2,
            -0.1]) {  
                cube([hor_cutout_width + 0.1 + tolerance, ver_width + tolerance, hor_span_height + 0.2]);
        }
        translate([
            hor_width - hor_cutout_width - tolerance,
            hor_length/2 - hor_cutout_distance/2 - ver_width/2 - tolerance/2,
            -0.1]) {
                cube([hor_cutout_width + 0.1 + tolerance, ver_width + tolerance, hor_span_height + 0.2]);
        }
        translate([
            -0.1,
            hor_length/2 + hor_cutout_distance/2 - ver_width/2 - tolerance/2,
            -0.1]) {
                cube([hor_cutout_width + 0.1 + tolerance, ver_width + tolerance, hor_span_height + 0.2]);
        }
        translate([
            hor_width - hor_cutout_width - tolerance,
            hor_length/2 + hor_cutout_distance/2 - ver_width/2 - tolerance/2,
            -0.1]) {
                cube([hor_cutout_width + 0.1 + tolerance, ver_width + tolerance, hor_span_height + 0.2]);
        }
    }
}


//
// Render Code
//
if (render_mode == 1) {
    vertical_mount();
}

if (render_mode == 2) {
    horizontal_mount();
}

if (render_mode == 3) {
    // Position horizontal mount at centerline of a virtual camera body
    translate([(tcam_mini_width - hor_width)/2, -wall_thickness, ver_span_height - hor_span_height]) {
        horizontal_mount();
    }
    
    // Position vertical mounts rotated to vertical and perpendicular to base
    translate([-wall_thickness, (tcam_mini_length - hor_cutout_distance + ver_width + tolerance)/2, ver_height]) {
        rotate([0, 90, 270]) {
            vertical_mount();
        }
    }
    
    translate([-wall_thickness, (tcam_mini_length + hor_cutout_distance + ver_width + tolerance)/2, ver_height]) {
        rotate([0, 90, 270]) {
            vertical_mount();
        }
    }
}