package org.lasarobotics.vision.opmode.extensions;

import org.lasarobotics.vision.detection.ColorBlobDetector;
import org.lasarobotics.vision.detection.objects.Contour;
import org.lasarobotics.vision.ftc.resq.Beacon;
import org.lasarobotics.vision.opmode.VisionOpMode;
import org.lasarobotics.vision.util.color.ColorHSV;
import org.opencv.core.Mat;

import java.util.List;

/**
 * Extension that supports finding and reading beacon color data
 */
public class BeaconExtension implements VisionExtension {
    private final ColorHSV lowerBoundRed = new ColorHSV((int) (305 / 360.0 * 255.0), (int) (0.100 * 255.0), (int) (0.300 * 255.0));
    private final ColorHSV upperBoundRed = new ColorHSV((int) ((360.0 + 5.0) / 360.0 * 255.0), 255, 255);
    private final ColorHSV lowerBoundBlue = new ColorHSV((int) (170.0 / 360.0 * 255.0), (int) (0.100 * 255.0), (int) (0.300 * 255.0));
    private final ColorHSV upperBoundBlue = new ColorHSV((int) (227.0 / 360.0 * 255.0), 255, 255);
    private ColorBlobDetector detectorRed;
    private ColorBlobDetector detectorBlue;

    private Beacon.BeaconAnalysis analysis = new Beacon.BeaconAnalysis();

    public Beacon.BeaconAnalysis getAnalysis() {
        return analysis;
    }

    public void init(VisionOpMode opmode) {
        //Initialize all detectors here
        detectorRed = new ColorBlobDetector(lowerBoundRed, upperBoundRed);
        detectorBlue = new ColorBlobDetector(lowerBoundBlue, upperBoundBlue);

        //opmode.setCamera(Cameras.PRIMARY);
        //opmode.setFrameSize(new Size(900, 900));
    }

    public void loop(VisionOpMode opmode) {

    }

    public Mat frame(VisionOpMode opmode, Mat rgba, Mat gray) {
        try {
            //Process the frame for the color blobs
            detectorRed.process(rgba);
            detectorBlue.process(rgba);

            //Get the list of contours
            List<Contour> contoursRed = detectorRed.getContours();
            List<Contour> contoursBlue = detectorBlue.getContours();

            //Get color analysis
            Beacon beacon = new Beacon();
            this.analysis = beacon.analyzeColor(contoursRed, contoursBlue, rgba, gray);

        } catch (Exception e) {
            e.printStackTrace();
        }

        return rgba;
    }

    @Override
    public void stop(VisionOpMode opmode) {

    }
}