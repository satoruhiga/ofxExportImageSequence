#include "ofxExportImageSequence.h"

#include "tiff.h"
#include "tiffio.h"

struct TiffExportOp
{
	enum Compression {
		UNCOMPRESSED = COMPRESSION_NONE,
		PACKBITS = COMPRESSION_PACKBITS,
		LZW = COMPRESSION_LZW
	};
	
	string path;
	ofPixels pix;
	
	int fd;
	TIFF* image;
	Compression compression;
	
	TiffExportOp(const string& path, const ofPixels& pix, Compression compression = PACKBITS)
	: path(path)
	, pix(pix)
	, compression(compression)
	, fd(0)
	, image(0)
	{}
	
	~TiffExportOp()
	{
		if (image) TIFFClose(image);
		image = NULL;
		
		if (fd) close(fd);
		fd = 0;
	}
	
	void operator()()
	{
		int width = pix.getWidth();
		int height = pix.getHeight();
		int bit_per_sample = 8;
		int byte_per_sample = bit_per_sample / 8;
		int sample_per_pixel = pix.getNumChannels();
		
		string path = ofToDataPath(this->path);
		
		fd = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
		image = TIFFFdOpen(fd, path.c_str(), "w");
		assert(image);
		
		TIFFSetField(image, TIFFTAG_IMAGEWIDTH, width);
		TIFFSetField(image, TIFFTAG_IMAGELENGTH, height);
		TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, sample_per_pixel);
		TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, bit_per_sample);
		TIFFSetField(image, TIFFTAG_COMPRESSION, compression);
		TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(image, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(image, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
		
		if (compression == COMPRESSION_LZW)
		{
			TIFFSetField(image, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
		}
		
		int rows_per_strip = TIFFDefaultStripSize(image, 0);
		TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, rows_per_strip);
		
		unsigned char* src = (unsigned char*)pix.getPixels();
		int strip_index = 0;
		
		for (int y = 0; y < height; y += rows_per_strip)
		{
			int remain = (height - y);
			int strip_size = remain > rows_per_strip ? rows_per_strip : remain;
			int num_bytes = TIFFVStripSize(image, strip_size);
			
			if (TIFFWriteEncodedStrip(image, strip_index++, src, num_bytes) < 0)
			{
				cout << "error on TIFFWriteEncodedStrip";
				break;
			}
			
			src += num_bytes;
		}
	}
};

struct TiffExportTask : public Poco::Task
{
	TiffExportOp op;
	
	TiffExportTask(const string& path,
		   const ofPixels& pix,
		   TiffExportOp::Compression compression)
		: Task(path)
		, op(path, pix, compression)
	{}
	
	void runTask()
	{
		op();
		ofLogNotice("ofxExportImageSequence") << "saved: " << name();
	}
};

void ofxExportImageSequence::saveImage(const ofPixels& pix, const string& path)
{
	TiffExportOp::Compression c;
	switch (compression) {
	  case UNCOMPRESSED:
			c = TiffExportOp::UNCOMPRESSED;
			break;
		case PACKBITS:
			c = TiffExportOp::PACKBITS;
			break;
		case LZW:
			c = TiffExportOp::LZW;
			break;
	}
	
	taskmanager.start(new TiffExportTask(path, pix, c));
}
