#pragma once

#include "ofMain.h"

#include "Poco/ThreadPool.h"
#include "Poco/TaskManager.h"

class ofxExportImageSequence
{
public:
	
	enum Compression {
		UNCOMPRESSED, // Fastest, super fat file size
		PACKBITS, // Good barance
		LZW // Same to ofSaveImage
	};

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
		, compression(PACKBITS)
		, threadpool(1)
		, taskmanager(threadpool)
	{}
	
	~ofxExportImageSequence()
	{
		threadpool.joinAll();
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
		s.useDepth = true;
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
			saveImage(pix, buf);

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
		
		ofDirectory::createDirectory(ofFile(pattern).getEnclosingDirectory(), true, true);
	}

	void stopExport() { do_export = false; }

	void setFrameRange(int in_frame, int out_frame = 0)
	{
		this->in_frame = in_frame;
		this->out_frame = out_frame;
	}

	void setOutputDir(const string& path)
	{
		export_path = path;
		this->pattern = path + "/" + path + "_%05i.tif";
	}

	const string& getFilePattern() const { return pattern; }

	void setOverwriteSequence(bool yn) { overwrite_sequence = yn; }
	bool getOverwriteSequence() const { return overwrite_sequence; }

	void setAutoExit(bool yn) { auto_exit = yn; }
	bool getAutoExit() const { return auto_exit; }
	
	void setCompression(Compression v) { compression = v; }

	//
	
	int getFrameNum() const { return frame_count; }
	
	float getElapsedTime() const
	{
		if (do_export)
			return frame_count * inv_fps;
		else
			return ofGetElapsedTimef();
	}
	
	float getLastFrameTime() const
	{
		if (do_export)
			return inv_fps;
		else
			return ofGetLastFrameTime();
	}

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
	
	Compression compression;
	
	Poco::ThreadPool threadpool;
	Poco::TaskManager taskmanager;
	
	void saveImage(const ofPixels& pix, const string& path);
};
