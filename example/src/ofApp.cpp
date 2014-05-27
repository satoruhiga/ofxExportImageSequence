#include "ofMain.h"

#include "ofxExportImageSequence.h"

class ofApp : public ofBaseApp
{
public:
	ofxExportImageSequence exp;

	void setup()
	{
		ofSetFrameRate(60);
		ofSetVerticalSync(true);
		ofBackground(0);

		exp.setup(1280, 720, 30);
		exp.setFrameRange(100, 200);
		exp.setOutputDir("out");

		exp.setOverwriteSequence(true);
		exp.setAutoExit(true);

		exp.startExport();
	}

	ofEasyCam cam;

	void update()
	{
		exp.begin(cam);
		ofClear(0);

		ofDrawBitmapString(ofToString(exp.getFrameNum()), 10, 20);

		exp.end();
	}

	void draw() { exp.draw(0, 0); }

	void keyPressed(int key) {}

	void keyReleased(int key) {}

	void mouseMoved(int x, int y) {}

	void mouseDragged(int x, int y, int button) {}

	void mousePressed(int x, int y, int button) {}

	void mouseReleased(int x, int y, int button) {}

	void windowResized(int w, int h) {}
};

#include "ofAppGLFWWindow.h"
int main(int argc, const char** argv)
{
	ofAppGLFWWindow window;
	window.setNumSamples(1);
	ofSetupOpenGL(&window, 1280, 720, OF_WINDOW);
	ofRunApp(new ofApp);
	return 0;
}
