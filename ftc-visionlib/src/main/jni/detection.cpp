//
// Implements advanced object detection methods
//

#include <jni.h>
#include <vector>
#include <stdio.h>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

extern "C"
{

int minHessian = 400;

Mat img_object;
Ptr<SURF> detector;

JNIEXPORT void JNICALL Java_com_lasarobotics_vision_detection_Detection_analyzeObject(JNIEnv* jobject, jlong addrGrayObject) {
    img_object = *(Mat*)addrGrayObject;

    if( !img_object.data )
    { printf(" --(!) Error reading images "); return; }

    //-- Step 1: Detect the keypoints using SURF Detector

    detector = SURF::create(minHessian);

    std::vector<KeyPoint> keypoints_object;
    detector->detect( img_object, keypoints_object );

    //-- Step 2: Calculate descriptors (feature vectors)
    Mat descriptors_object;
    detector->compute( img_object, keypoints_object, descriptors_object );
}

JNIEXPORT void JNICALL Java_com_lasarobotics_vision_detection_Detection_findObject(JNIEnv* jobject, jlong addrGrayObject, jlong addrGrayScene, jlong addrOutput) {

    Mat img_scene = *(Mat*)addrGrayScene;
    Mat img_matches = *(Mat*)addrOutput;

    if( !img_object.data || !img_scene.data )
    { printf(" --(!) Error reading images "); return; }

    //-- Step 1: Detect the keypoints using SURF Detector
    Ptr<SURF> detector = SURF::create(minHessian);

    std::vector<KeyPoint> keypoints_scene;
    detector->detect( img_scene, keypoints_scene );

    //-- Step 2: Calculate descriptors (feature vectors)
    //Ptr<SURF> extractor = SURF::create();

    Mat descriptors_scene;
    detector->compute( img_scene, keypoints_scene, descriptors_scene );

    //-- Step 3: Matching descriptor vectors using FLANN matcher
    FlannBasedMatcher matcher;
    std::vector< DMatch > matches;
    matcher.match( descriptors_object, descriptors_scene, matches );

    double max_dist = 0; double min_dist = 100;

    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_object.rows; i++ )
    { double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

    printf("-- Max dist : %f \n", max_dist );
    printf("-- Min dist : %f \n", min_dist );

    //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
    std::vector< DMatch > good_matches;

    for( int i = 0; i < descriptors_object.rows; i++ )
    { if( matches[i].distance < 3*min_dist )
        { good_matches.push_back( matches[i]); }
    }

    //drawMatches( img_object, keypoints_object, img_scene, keypoints_scene,
    //             good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
    //             vector<char>(), DrawMatchesFlags::DRAW_OVER_OUTIMG);

    //-- Localize the object
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;

    for( int i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
    }

    Mat H = findHomography( obj, scene, RANSAC );

    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( img_object.cols, 0 );
    obj_corners[2] = cvPoint( img_object.cols, img_object.rows ); obj_corners[3] = cvPoint( 0, img_object.rows );
    std::vector<Point2f> scene_corners(4);

    perspectiveTransform( obj_corners, scene_corners, H);

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
    line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar(0, 255, 0), 4 );
    line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );

    //-- Show detected matches
    //imshow( "Good Matches & Object detection", img_matches );

    return;
}

} //extern "C"