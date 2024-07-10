#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){

	color = ofColor(0, ofRandom(80, 175), 0, 255);

	sound.load("HexagonalDepth.wav");
	sound.setVolume(0.25);
	sound.setLoop(true);

	pixile.Master(false);
	pixile.Computer_id(1);
	pixile.Server_port(3637);
	pixile.SetupSockets();
	pixile.start();
}

//--------------------------------------------------------------
void ofApp::update(){
	pixile.update();

	if (pixile.SoundsOn() != soundOn) {
		soundOn = pixile.SoundsOn();
		if (soundOn) {
			if (!sound.isPlaying()) sound.play();
		}
		else {
			sound.stop();
		}
	}

	if(ofGetFrameNum()%4==0) color.setHue(color.getHue() + 1);
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofSetColor(color);
	ofFill();
	ofDrawRectangle(0,0,ofGetWidth(),ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
