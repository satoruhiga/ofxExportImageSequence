#pragma once

#include "ofMain.h"

class ofxExportImageSequence
{
public:
	ofxExportImageSequence()
		: cam_ptr(NULL)
		, width(0)
		, height(0)
		, do_export(false)
		, fps(0)
		, inv_fps(0)
		, in_frame(0)
		, out_frame(0)
		, overwrite_sequence(false)
		, auto_exit(false)
	{
	}

	void setup(int width, int height, float fps, GLint internalformat = GL_RGB,
			   int num_samples = 0)
	{
		this->width = width;
		this->height = height;
		this->fps = fps;
		this->inv_fps = 1. / fps;

		setOutputDir("export");

		ofFbo::Settings s;
		s.width = width;
		s.height = height;
		s.internalformat = internalformat;
		s.numSamples = num_samples;
		fbo.allocate(s);

		fbo.begin();
		ofClear(0);
		fbo.end();
	}

	//

	void begin(ofCamera& cam)
	{
		ofPushView();
		fbo.begin();

		cam_ptr = &cam;
		cam_ptr->begin(ofRectangle(0, 0, width, height));
	}

	void begin()
	{
		ofPushView();
		fbo.begin();

		ofViewport(0, 0, width, height);

		cam_ptr = NULL;
	}

	void end()
	{
		if (cam_ptr)
		{
			cam_ptr->end();
			cam_ptr = NULL;
		}

		fbo.end();
		ofPopView();

		if (do_export)
		{
			char buf[256];
			sprintf(buf, pattern.c_str(), frame_count);

			ofPixels pix;
			fbo.readToPixels(pix);
			ofSaveImage(pix, buf);

			ofLogNotice("ofxExportImageSequence") << "saved: " << buf;

			frame_count++;

			if (out_frame > 0 && frame_count >= out_frame)
			{
				stopExport();

				if (auto_exit)
				{
					ofExit(0);
				}
			}
			
			stringstream ss;
			
			if (out_frame > 0)
			{
				float pct = (float)(frame_count - in_frame) / (out_frame - in_frame);
				ss << "progress: " << ofToString(pct * 100, 0) << "% - ";
			}
			
			ss << ofToString(ofGetFrameRate(), 1) << "fps";
			ofSetWindowTitle(ss.str());
		}
	}

	void draw(float x, float y, float w = 0, float h = 0)
	{
		ofRectangle r(0, 0, fbo.getWidth(), fbo.getHeight());

		if (w == 0) w = ofGetWidth();
		if (h == 0) h = ofGetHeight();

		r.scaleTo(ofRectangle(0, 0, w, h));

		fbo.draw(r);
	}

	//
	
	bool isExporting() const { return do_export; }

	void startExport()
	{
		do_export = true;

		frame_count = in_frame;

		if (overwrite_sequence)
		{
			string path = ofFilePath::getEnclosingDirectory(pattern);
			ofDirectory::removeDirectory(path, true);
		}
	}

	void stopExport() { do_export = false; }

	void setFrameRange(int in_frame, int out_frame = 0)
	{
		this->in_frame = in_frame;
		this->out_frame = out_frame;
	}

	void setOutputDir(const string& path, const string& extension = "tif")
	{
		export_path = path;
		setFilePattern(path + "/" + path + "_%05i." + extension);
	}

	void setFilePattern(const string& pattern) { this->pattern = pattern; }

	const string& getFilePattern() const { return pattern; }

	void setOverwriteSequence(bool yn) { overwrite_sequence = yn; }
	bool getOverwriteSequence() const { return overwrite_sequence; }

	void setAutoExit(bool yn) { auto_exit = yn; }
	bool getAutoExit() const { return auto_exit; }

	//

	int getFrameNum() const { return frame_count; }
	float getElapsedTime() const { return frame_count * inv_fps; }

	ofFbo& getFbo() { return fbo; }

protected:
	bool do_export;
	bool overwrite_sequence;
	bool auto_exit;

	int width, height;

	int frame_count;

	float fps;
	float inv_fps;

	int in_frame, out_frame;

	string pattern;
	string export_path;

	ofFbo fbo;
	ofCamera* cam_ptr;
};
