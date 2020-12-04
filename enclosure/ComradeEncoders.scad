showTopPanel = false;
showBottomLid = true;

holeDiam = 3.2;

holeDistXR = 3.4;
holeDistXL = 3.48;
holeDistY = 3;

lipX = 10;
lipY = 10;

marginXL = 6.2;
marginXR = 2;
marginY = 2.5;

thickness = 3;
sideWallThickness = 4;

usbJackWidth = 12;
usbJackHeight = 11;
usbJackBottomOffset = 12;

caseHeight = 30;

dispOuterX = 69.2;
dispOuterY = 50;

d = 1;

$fn = 360;

encoderHoleDiam = 6.83;
encoderSpacing = 30;

encoderNumX = 4;
encoderNumY = 2;

sideScrewHoleDiam = 2.7;
sideScrewHoleZOffset = 2.5;
sideScrewHoleYOffset = 15;

caseSizeX = max(dispOuterX + 2 * lipX, encoderNumX * encoderSpacing);
caseSizeY = dispOuterY + 2 * lipY + encoderNumY * encoderSpacing;

encoderAreaCoordY = - dispOuterY / 2 - lipY;

screwHoleZOffset = 2.5;
screwHoleDiam = 2.7;
screwHoleYOffset = 10;

lidThickness = 2;
lidLipHeight = 1;
lidLipThickness = 2;
lidLipCornerHeight = screwHoleZOffset * 2;
lidLipCornerLength = (screwHoleYOffset - sideWallThickness) * 2;
lidLipTaperLength = 5;
lidLipTaperHeight = lidLipCornerHeight - lidLipHeight;

module displayScrewHole(x, y) {
    color("red")
        translate([x, y, 0])
            cylinder(h = thickness + 2 * d, d = holeDiam, center = true);
}

module encoderHole(x, y) {
    color("green")
        translate([x, y, 0])
            cylinder(h = thickness + 2 * d, d = encoderHoleDiam, center = true);
}

module topPanel()
{
    difference() {
        translate([0, -encoderSpacing, 0])
            cube(
                [
                    caseSizeX,
                    caseSizeY,
                    thickness
                ],
                center = true
            );
        
        // Display window
        color("blue")
            cube(
                [
                    dispOuterX - marginXL - marginXR,
                    dispOuterY - 2 * marginY,
                    thickness + 2 * d
                ],
                center = true
            );
        
        // Display screw holes
        translate([(marginXR - marginXL) / 2, 0, 0])
            for (i = [-1: 2: 1]) {
                displayScrewHole(
                    dispOuterX / 2 + holeDistXR,
                    i * (dispOuterY / 2 - holeDistY)
                );
                
                displayScrewHole(
                    - dispOuterX / 2 - holeDistXL,
                    i * (dispOuterY / 2 - holeDistY)
                );
            }
        
        // Encoder holes
        translate([0, encoderAreaCoordY - encoderSpacing / 2, 0])
        for (i = [(1 - encoderNumX) / 2 : (encoderNumX - 1) / 2]) {
            for (j = [0, 1 - encoderNumY]) {
                encoderHole(encoderSpacing * i, encoderSpacing * j);
            }
        }
    }
}

module sideWalls() {
    translate([
        - caseSizeX / 2,
        -caseSizeY,
        -caseHeight
    ])
        cube([sideWallThickness, caseSizeY, caseHeight]);

    translate([
        caseSizeX / 2 - sideWallThickness,
        -caseSizeY,
        -caseHeight
    ])
        cube([sideWallThickness, caseSizeY, caseHeight]);

    translate([
        - caseSizeX / 2,
        -caseSizeY,
        -caseHeight
    ])
        cube([caseSizeX, sideWallThickness, caseHeight]);
        
    translate([
        - caseSizeX / 2,
        - sideWallThickness,
        -caseHeight
    ])
        cube([caseSizeX, sideWallThickness, caseHeight]);
}

module lidTaperPrism() {
    rotate(-90, [0, 1, 0])
        linear_extrude(height = sideWallThickness)
            polygon([[0, 0], [0, lidLipTaperLength], [lidLipTaperHeight, 0]]);
}

module taperedCorner() {
    // lip corner
    color("Red")
        cube([
            lidLipCornerLength,
            lidLipCornerLength,
            lidLipCornerHeight
        ]);
    
    // lip tapers
    color("Green") {
        translate([
            sideWallThickness,
            lidLipCornerLength,
            lidLipHeight
        ])
            lidTaperPrism();
        
        translate([
            lidLipCornerLength,
            0,
            lidLipHeight
        ])
            rotate(-90, [0, 0, 1])
                lidTaperPrism();
    }
}

module twoTaperedCorners() {
    translate([sideWallThickness, sideWallThickness, 0]) {
        taperedCorner();
        
        translate([caseSizeX - 2 * sideWallThickness, 0, 0])
            mirror([1, 0, 0])
                taperedCorner();
    }
}

module bottomLid() {
    translate([0, 0, -lidThickness])
        cube([caseSizeX, caseSizeY, lidThickness]);
    
    // now lip
    difference() {
        union() {
            // base lip
            color("Cyan")
                translate([sideWallThickness, sideWallThickness, 0])
                    cube([
                        caseSizeX - 2 * sideWallThickness,
                        caseSizeY - 2 * sideWallThickness,
                        lidLipHeight
                    ]);
            
            
            // tapered corners
            twoTaperedCorners();
            
            translate([0, caseSizeY, 0])
                mirror([0, 1, 0])
                    twoTaperedCorners();
        }
        
        color("Cyan")
            translate([
                sideWallThickness + lidLipThickness,
                sideWallThickness + lidLipThickness,
                - d
            ])
                cube([
                    caseSizeX - 2 * (sideWallThickness + lidLipThickness),
                    caseSizeY - 2 * (sideWallThickness + lidLipThickness),
                    lidLipCornerHeight + 2 *d
                ]);
        
        // Screw holes
        translate([
            caseSizeX / 2, 0, caseHeight
        ]) {
            sideScrewHole(sideScrewHoleYOffset);
            sideScrewHole(caseSizeY - sideScrewHoleYOffset);
        }
    }
}

if (showTopPanel) {
translate([0, encoderAreaCoordY, thickness / 2])
    topPanel();
}

if (showBottomLid) {
    translate([- caseSizeX / 2, -caseSizeY, -caseHeight])
        bottomLid();
}


module sideScrewHole(y)
{
    color("magenta") {
        translate([
            - caseSizeX / 2 - d,
            y,
            - (caseHeight - sideScrewHoleZOffset)
        ])
            rotate(90, [0, 1, 0])
                cylinder(d = sideScrewHoleDiam, h = caseSizeX + 2 * d);
    }
}

if (showTopPanel) {
    difference() {
        color("purple")
            sideWalls();
        
        // USB jack hole
        translate([
            0,
            - sideWallThickness / 2,
            - (caseHeight - usbJackBottomOffset)
        ])
            color("cyan")
                cube(
                    [
                        usbJackWidth,
                        sideWallThickness + 2 * d,
                        usbJackHeight
                    ],
                    center = true
                );
        
        sideScrewHole(- sideScrewHoleYOffset);
        sideScrewHole(- (caseSizeY - sideScrewHoleYOffset));
    }
}
