#include "Resources.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/DisplayList.h"
#include "cinder/gl/gl.h"
#include "cinder/Xml.h">
//#include "cinder/params/Params.h"
#include "cinder/MayaCamUI.h"
//#include "cinder/Arcball.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define PI 3.14159265

int sphereDetail = 5;

class ParticleGlobeApp : public AppBasic {
public:
	void prepareSettings(Settings *settings);
	void setup();
	//void resize( ResizeEvent event);
	void mouseDown(MouseEvent event);
	void mouseDrag(MouseEvent event);
	void plotPoint(Vec2f location);
	void update();
	void draw();
	void setupSlerp();
	void drawPathBetweenPoints(Vec2f, Vec2f);

	// PARAMS
	//params::InterfaceGl	mParams;
	//Arcball		mArcball;
	Vec2i		mInitialMouseDown, mCurrentMouseDown;

	//CAMERA
	CameraPersp		mCam;
	Quatf			mSceneRotation;
	Vec3f			mVecA, mVecB;
	MayaCamUI		mMayaCam;

	//SPHERE
	float			r, x, y, z, mXRes, mYRes, mAlpha;
	int				mTotal;
	gl::DisplayList	mGlobe, mQuake, mCity;

	//TEXTURE
	Channel32f		mChannel;
	gl::Texture		mTexture;


};

void ParticleGlobeApp::prepareSettings(Settings *settings)
{
	settings->setWindowSize(800, 600);
	settings->setFrameRate(60.0f);
	settings->setFullScreen(false);
}

/*void ParticleGlobeApp::resize( ResizeEvent event )
{
//mArcball.setWindowSize( getWindowSize() );
//mArcball.setCenter( getWindowCenter() );
//mArcball.setRadius( 200 );
}*/

void ParticleGlobeApp::setup()
{
	glFrontFace(GL_CW); // the default camera inverts to a clockwise front-facing direction

						//mArcball.setConstraintAxis(Vec3f(0,0,1));

	CameraPersp cam;
	cam.setEyePoint(Vec3f(0, 0, 1000));
	cam.setCenterOfInterestPoint(Vec3f::zero());
	cam.setPerspective(60.0, getWindowAspectRatio(), 1.0, 3000.0);
	mMayaCam.setCurrentCam(cam);

	mChannel = Channel32f(loadImage(loadFile("/Users/jed.bursiek/Experiments/Cinder/projects/ParticleGlobe/data/color_map.png")));
	//mTexture = mChannel;

	float latitude, longitude;

	r = 450;
	//mTotal = 100;
	mXRes = mChannel.getWidth();
	mYRes = mChannel.getHeight();

	console() << mXRes << " " << mYRes;

	mGlobe = gl::DisplayList(GL_COMPILE);
	mGlobe.newList();

	for (int i = 0; i<mYRes; i += 1) {
		for (int j = 0; j<mXRes; j += 1) {


			//c++ uses radians
			latitude = (mYRes / 2 - i)*PI / 180;
			longitude = -(mXRes / 2 - j)*PI / 180;

			//math to map lat and long to a globe
			x = r * cos(latitude) * cos(longitude);
			y = r * cos(latitude) * sin(longitude);
			z = r * sin(latitude);

			mAlpha = mChannel.getValue(Vec2f(j, i));
			//console() << "mAlpha :: " << mAlpha << std::endl;
			//if(mAlpha >= 0.1){
			gl::drawSphere(Vec3f(x, y, z), 2 * mAlpha, sphereDetail);
			//}
		}
	}

	mGlobe.endList();

	/**
	*Quake Data
	*/

	XmlTree doc(loadFile("/Users/jed.bursiek/Experiments/Cinder/projects/ParticleGlobe/data/earthquake_data.xml"));
	XmlTree data = doc.getChild("data/");

	mQuake = gl::DisplayList(GL_COMPILE);
	mQuake.newList();

	for (XmlTree::Iter quake = data.begin(); quake != data.end(); ++quake) {

		//gather all the xml data	
		float lat = quake->getChild("lat/").getValue<float>();
		float lon = quake->getChild("lon/").getValue<float>();
		//float mag = quake->getChild("mag/").getValue<float>();
		float depth = quake->getChild("depth/").getValue<float>();

		//need to figure out the radius of earht in feet to apply correct ratio
		//depth = depth*0.1;
		depth = 0.0f;
		//c++ uses radians
		latitude = lat*PI / 180;
		longitude = lon*PI / 180;

		//math to map lat and long to a globe
		x = (r + depth) * cos(latitude) * cos(longitude);
		y = (r + depth) * cos(latitude) * sin(longitude);
		z = (r + depth) * sin(latitude);

		gl::drawSphere(Vec3f(x, y, z), 2.0, sphereDetail);

	}

	mQuake.endList();

	/**
	*Citys Data
	*/

	XmlTree world(loadFile("/Users/jed.bursiek/Experiments/Cinder/projects/ParticleGlobe/data/world_cities.xml"));
	XmlTree city = world.getChild("data/");

	mCity = gl::DisplayList(GL_COMPILE);
	mCity.newList();

	for (XmlTree::Iter place = city.begin(); place != city.end(); ++place) {

		//gather all the xml data	
		float lat = place->getChild("lat/").getValue<float>();
		float lon = place->getChild("lon/").getValue<float>();
		float pop = place->getChild("pop/").getValue<float>();
		//float depth = place->getChild("depth/").getValue<float>();

		//need to figure out the radius of earht in feet to apply correct ratio
		//depth = depth*0.001;
		pop = pop*0.000001;

		//c++ uses radians
		latitude = lat*PI / 180;
		longitude = lon*PI / 180;

		//math to map lat and long to a globe
		x = r * cos(latitude) * cos(longitude);
		y = r * cos(latitude) * sin(longitude);
		z = r * sin(latitude);

		gl::drawSphere(Vec3f(x, y, z), 1.0, sphereDetail);

	}

	mCity.endList();

	/*for( int i=0; i<mXRes; i+=1 ){
	for( int j=0; j<mYRes; j+=1 ){

	latitude = mYRes/2 - i;
	longitude = mXRes/2 - j;
	//latitude = i;
	//longitude = j;

	x = longitude;
	y = latitude;
	z = 20;
	mAlpha = mChannel.getValue(Vec2f(i,j));
	console() << "mAlpha :: " << mAlpha << std::endl;
	if(mAlpha >= 0.5){
	gl::drawSphere( Vec3f(x, y, z), mAlpha, sphereDetail );
	}
	}
	}*/



	//parameter (track ball)
	//mParams = params::InterfaceGl( "SphereWorld", Vec2i( 225, 200 ) );
	//mParams.addParam( "Scene Rotation", &mSceneRotation, "opened=1");

	setupSlerp();
}

void ParticleGlobeApp::plotPoint(Vec2f location)
{
	float lat, lon;
	lat = location.x*PI / 180;
	lon = location.y*PI / 180;

	//math to map lat and long to a globe
	x = r * cos(lat) * cos(lon);
	y = r * cos(lat) * sin(lon);
	z = r * sin(lat);

	glColor4f(ColorA(1.0f, 0.0f, 0.0f, 1.0f));
	gl::drawSphere(Vec3f(x, y, z), 5.0, sphereDetail);
}

void ParticleGlobeApp::mouseDown(MouseEvent event)
{
	//arcball rotates the globe specifically
	//mArcball.mouseDown( event.getPos() );
	//mCurrentMouseDown = mInitialMouseDown = event.getPos();

	//maya cam rotates the entire scene
	mMayaCam.mouseDown(event.getPos());
}

void ParticleGlobeApp::mouseDrag(MouseEvent event)
{
	//arcball rotates the globe specifically
	//mArcball.mouseDrag(event.getPos());
	//mCurrentMouseDown = event.getPos();

	//maya cam rotates the entire scene
	mMayaCam.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
}

void ParticleGlobeApp::update()
{
	//mCam.lookAt( Vec3f( 0.0f, 0.0f, 1000.0f ), Vec3f::zero(), Vec3f::yAxis());
	//mCam.setPerspective( 60.0f, getWindowAspectRatio(), 5.0f, 3000.0f );
	//gl::setMatrices( mCam );
	//gl::rotate( mSceneRotation );

	gl::setMatrices(mMayaCam.getCamera());
}

void ParticleGlobeApp::setupSlerp()
{
	mVecA = Vec3f(450, 0, 0);//randVec3f();
	mVecB = Vec3f(0, 450, 0);//randVec3f();
							 //mSlerpAmt = 0;
}

void ParticleGlobeApp::drawPathBetweenPoints(Vec2f point1, Vec2f point2)
{
	//c++ uses radians
	Vec2f pointA = point1*PI / 180;
	Vec2f pointB = point2*PI / 180;

	//console() << point1 << endl;

	float x1 = r * cos(pointA.x) * cos(pointA.y);
	float y1 = r * cos(pointA.x) * sin(pointA.y);
	float z1 = r * sin(pointA.x);

	Vec3f a = Vec3f(x1, y1, z1);

	float x2 = r * cos(pointB.x) * cos(pointB.y);
	float y2 = r * cos(pointB.x) * sin(pointB.y);
	float z2 = r * sin(pointB.x);

	Vec3f b = Vec3f(x2, y2, z2);

	console() << a << " " << b << endl;

	// draws a path composed of 100 line segments
	glBegin(GL_LINE_STRIP);
	for (float t = 0; t <= 1.0f; t += 0.01f)
		gl::vertex(a.slerp(t, b));
	glEnd();
}

void ParticleGlobeApp::draw()
{
	// clear out the window with black
	gl::clear(Color(0, 0, 0));

	//mTexture.enableAndBind();
	//gl::draw( mTexture, getWindowBounds() );

	gl::enableDepthRead();
	gl::enableDepthWrite();

	//glEnable (GL_BLEND);

	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glDisable(GL_TEXTURE_2D);
	// DRAW PARTICLES

	//gl::pushModelView();
	//gl::rotate( mArcball.getQuat() );
	//ocean color (blue)
	//glColor4f( ColorA( 0.0f, 11.0f, 29.0f, 64.0.0f )/255.0f );
	//gl::drawSphere( Vec3f(0, 0, 0), r - 5.0, 100 );
	//land color (green)
	glColor4f(ColorA(0.0f, 148.0f, 77.0f, 255.0f) / 255.0f);
	mGlobe.draw();
	glColor4f(ColorA(255.0f, 210.0f, 0.0f, 255.0f) / 255.0f);
	mQuake.draw();
	glColor4f(ColorA(255.0f, 255.0f, 255.0f, 255.0f) / 255.0f);
	mCity.draw();
	glColor4f(1.0, 1.0, 1.0, 1.0);
	gl::drawVector(Vec3f(0, 0, r), Vec3f(0, 0, r + 50), 0, 0);
	gl::drawVector(Vec3f(0, 0, -r), Vec3f(0, 0, -r - 50), 0, 0);

	// draw the path
	gl::color(Color::white());
	drawPathBetweenPoints(Vec2f(45.31, -122.40), Vec2f(19.13, -99.40));
	plotPoint(Vec2f(45.31, -122.40));//portland
	plotPoint(Vec2f(19.13, -99.40));//mexico city
									//plotPoint(Vec2f(31.2, 121.50));//shanghai
									//gl::popModelView();

									// DRAW PARAMS WINDOW
									//params::InterfaceGl::draw();
}


CINDER_APP(ParticleGlobeApp, RendererGl)
