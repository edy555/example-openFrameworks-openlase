#include "testApp.h"
#include "libol.h"

class Tracker
{
public:
	
	const ofxBvhJoint *joint;
	deque<ofVec3f> points;
	
	void setup(const ofxBvhJoint *o)
	{
		joint = o;
	}
	
	void update()
	{
		if (joint->getBvh()->isFrameNew())
		{
			const ofVec3f &p = joint->getPosition();
			
			points.push_front(joint->getPosition());
			
			if (points.size() > 15)
				points.pop_back();
		}
	}
	
	void draw()
	{
		if (points.empty()) return;
		
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < points.size() - 1; i++)
		{
			float a = ofMap(i, 0, points.size() - 1, 1, 0, true);
			
			ofVec3f &p0 = points[i];
			ofVec3f &p1 = points[i + 1];
			
			float d = p0.distance(p1);
			a *= ofMap(d, 3, 5, 0, 1, true);
			
			glColor4f(1, 1, 1, a);
			glVertex3fv(points[i].getPtr());
		}
		glEnd();
	}
};

vector<Tracker*> trackers;
const float trackDuration = 64.28;

//--------------------------------------------------------------
void testApp::setup()
{	
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	
	ofBackground(0);
	
	play_rate = play_rate_t = 1;
	rotate = 0;

	bvh.resize(3);
	
	// You have to get motion and sound data from http://www.perfume-global.com
	
	// setup bvh
	bvh[0].load("bvhfiles/aachan.bvh");
	bvh[1].load("bvhfiles/kashiyuka.bvh");
	bvh[2].load("bvhfiles/nocchi.bvh");
	bvh[0].setName("AACHAN");
	bvh[1].setName("KASHIYUKA");
	bvh[2].setName("NOCCHI");
	
	for (int i = 0; i < bvh.size(); i++)
	{
		bvh[i].setFrame(1);
	}
	
	track.loadSound("Perfume_globalsite_sound.wav");
	track.play();
	track.setLoop(true);
	
	// setup tracker
	for (int i = 0; i < bvh.size(); i++)
	{
		ofxBvh &b = bvh[i];
		
		for (int n = 0; n < b.getNumJoints(); n++)
		{
			const ofxBvhJoint *o = b.getJoint(n);
			Tracker *t = new Tracker;
			t->setup(o);
			trackers.push_back(t);
		}
	}
	
#if 1
	// setup OpenLase
	OLRenderParams params;
	
	memset(&params, 0, sizeof params);
	params.rate = 48000;
	params.on_speed = 2.0/100.0;
	params.off_speed = 2.0/20.0;
	params.start_wait = 8;
	params.start_dwell = 3;
	params.curve_dwell = 0;
	params.corner_dwell = 8;
	params.curve_angle = cosf(30.0*(M_PI/180.0)); // 30 deg
	params.end_dwell = 3;
	params.end_wait = 7;
	params.snap = 1/100000.0;
	params.render_flags = RENDER_GRAYSCALE;
	params.flatness = 1;
	
	params.start_wait = 20;
	//params.end_wait = 15;
	params.start_dwell = 15;
	//params.end_dwell = 8;
	//params.corner_dwell = 12;
	
	params.on_speed = 2.0/200.0;
	params.off_speed = 2.0/40.0;
	
	if(olInit(3, 30000) < 0) {
		ofLogError("testApp", "fail to initialize openlase");
	}
	
	olSetRenderParams(&params);
#endif
}

//--------------------------------------------------------------
void testApp::update()
{
	rotate += 0.1;
	
	play_rate += (play_rate_t - play_rate) * 0.3;
	track.setSpeed(play_rate);
	
	float t = (track.getPosition() * trackDuration);
	t = t / bvh[0].getDuration();
	
	for (int i = 0; i < bvh.size(); i++)
	{
		bvh[i].setPosition(t);
		bvh[i].update();
	}
	
	for (int i = 0; i < trackers.size(); i++)
	{
		trackers[i]->update();
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	glEnable(GL_DEPTH_TEST);
	
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		
	ofPushMatrix();
	{
		cam.begin();
#if 0		
		ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
		ofTranslate(0, 150);
		
		ofRotate(-15, 1, 0, 0);
		ofRotate(rotate, 0, 1, 0);
		
		ofScale(1, -1, 1);
#endif				
		ofSetColor(ofColor::white);

		ofFill();
		
		olLoadIdentity3();
		olLoadIdentity();
		olPerspective(60, 1, 1, 100);
		
		//olTranslate3(0, -1, -4);
		//olScale3(0.01, 0.01, 0.01);		
		olMultMatrix3(cam.getModelViewMatrix().getPtr());
		
		// draw ground
		ofPushMatrix();
		ofRotate(90, 1, 0, 0);
		ofLine(100, 0, -100, 0);
		ofLine(0, 100, 0, -100);
		ofPopMatrix();
		
		olTranslate3(0, -100, 0);
		olPushMatrix3();
		olBegin(OL_LINESTRIP);
		olVertex3(-100, 0, 0, C_WHITE);
		olVertex3(100, 0, 0, C_WHITE);
		olEnd();
		olBegin(OL_LINESTRIP);
		olVertex3(0, 0, -100, C_WHITE);
		olVertex3(0, 0, 100, C_WHITE);
		olEnd();
		olPopMatrix3();
		
		// draw actor
		for (int i = 0; i < bvh.size(); i++)
		{
			bvh[i].draw();
			cout << " ";
		}

		// draw tracker
		glDisable(GL_DEPTH_TEST);
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		
		ofSetColor(ofColor::white, 80);
		for (int i = 0; i < trackers.size(); i++)
		{
			trackers[i]->draw();
		}
		
		cam.end();
	}
	ofPopMatrix();
	
	ofSetColor(255);
	ofDrawBitmapString("press any key to scratch\nplay_rate: " + ofToString(play_rate, 1), 10, 20);
	
	cout << "\n";
	olRenderFrame(60);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	play_rate_t = -1;
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	play_rate_t = 1;
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
}
