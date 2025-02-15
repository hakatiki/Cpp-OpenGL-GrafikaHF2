//=============================================================================================
// Mintaprogram: Z�ld h�romsz�g. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Takats Balint
// Neptun : PUWI1T
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

//=============================================================================================
// Computer Graphics Sample Program: Ray-tracing-let
//=============================================================================================
const float YPOS = 0;
const float XPOS = 0;
const float Z = -1.5;
const float A = 0.09;
const float C = 0.4;
const float TOP = 1.2;
const float BOT = 0.95;
const vec3 topHoleCenter = vec3(XPOS, YPOS, BOT);
const int RESOLUTION = 20;
const float RAD = sqrt((1 + (BOT - Z) * (BOT - Z) / C / C)* A* A);
const float RAD_SQUARE = RAD * RAD;
const float AREA = RAD * RAD * 3.14515;
float deltaAREA = AREA / (float)RESOLUTION;



enum MaterialType{ROUGH, REFLECTIVE};
struct Material {
	vec3 ka, kd, ks;
	float  shininess;
	vec3 F0;
	MaterialType type;

	Material(MaterialType t) { type = t; }
};
struct RoughMaterial : Material {
	RoughMaterial(vec3 _kd, vec3 _ks, float _shininess) : Material(ROUGH) {
		ka = _kd * M_PI;
		kd = _kd;
		ks = _ks;
		shininess = _shininess;  
	}
};
vec3 operator/(vec3 num, vec3 den) {
	return vec3(num.x / den.x, num.y / den.y, num.z / den.z);
}
struct ReflectiveMaterial : Material {
	ReflectiveMaterial(vec3 n, vec3 kappa) : Material(REFLECTIVE){
		vec3 one(1, 1, 1);
		F0 = ((n - one) * (n - one) + kappa * kappa) / ((n + one) * (n + one) + kappa * kappa);
	}
};
struct Hit {
	float t;
	vec3 position, normal;
	Material* material;
	Hit() { t = -1; }
};

struct Ray {
	vec3 start;
	vec3 dir;
	Ray(vec3 _start, vec3 _dir) {
		start = _start;
		dir = normalize(_dir);
	}
};

class Intersectable {
protected:
	Material* material;
public:
	virtual Hit intersect(const Ray& ray) = 0;
};

inline vec4 toVec4(vec3 a, float b ){
	return vec4(a.x, a.y, a.z, b);
}
struct Elipsoid : public Intersectable {
	vec3 center;
	float radius;
	mat4 Q;

	Elipsoid(const vec3& _center, float a,float b, float c, Material* _material) {
		center = _center;
		float x = (-2.0f * _center.x) / (a * a);
		float x2 = _center.x * _center.x / (a * a);
		float y = (-2.0f * _center.y) / (b * b);
		float y2 = _center.y * _center.y / (b * b);
		float z = (-2.0f * _center.z) / (c * c);
		float z2 = _center.z * _center.z / (c * c);
		Q = mat4(
			1.0f / (a * a), 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f / (b * b), 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f / (c * c), 0.0f,
			x, y, z, -1.0f + x2 + y2 + z2
		);
		material = _material;
	}

	Hit intersect(const Ray& ray) {
		Hit hit;
		vec4 rayDir = toVec4(ray.dir, 0);
		vec4 rayStart = toVec4(ray.start, 1);
		float a = dot(rayDir * Q, rayDir);
		float b = dot(rayDir * Q, rayStart) + dot(rayStart * Q, rayDir);
		float c = dot(rayStart * Q, rayStart);

		float discr = b * b - 4.0f * a * c;
		if (discr < 0) return hit;
		float sqrt_discr = sqrtf(discr);
		float t1 = (-b + sqrt_discr) / 2.0f / a;	// t1 >= t2 for sure
		float t2 = (-b - sqrt_discr) / 2.0f / a;
		if (t1 <= 0) return hit;
		hit.t = (t2 > 0) ? t2 : t1;

		hit.position = ray.start + ray.dir * hit.t;
		vec4 gradf = toVec4(hit.position * 2.0f, 1.0) * Q;
		hit.normal = normalize(vec3(gradf.x, gradf.y, gradf.z));
		hit.material = material;
		return hit;

	}
};

struct Cylinder : public Intersectable {
	mat4 Q;
	vec3 center;
	float a;
	float b;
	float bot;
	float top;
	Cylinder(const vec3& _center, float _a, float _b,float _bot, float _top, Material* _material) {
		center = _center;
		a = _a;
		b = _b;
		bot = _bot;
		top = _top;
		float x = (-2.0f * _center.x) / (a * a);
		float x2 = _center.x * _center.x / (a * a);
		float y2 = _center.y * _center.y / (b * b);
		float y = (-2.0f * _center.y) / (b * b);
		Q = mat4(
			1.0f / (a * a), 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f / (b * b), 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			x, y, 0.0f, -1.0f + x2 + y2
		);
		material = _material;
	}
	Hit intersect(const Ray& ray) {
		Hit hit;
		vec4 rayDir = toVec4(ray.dir, 0);
		vec4 rayStart = toVec4(ray.start, 1);
		float a = dot(rayDir * Q, rayDir);
		float b = dot(rayDir * Q, rayStart) + dot(rayStart * Q, rayDir);
		float c = dot(rayStart * Q, rayStart);

		float discr = b * b - 4.0f * a * c;
		if (discr < 0) return hit;
		float sqrt_discr = sqrtf(discr);
		float t1 = (-b + sqrt_discr) / 2.0f / a;	// t1 >= t2 for sure
		float t2 = (-b - sqrt_discr) / 2.0f / a;
		if (t1 <= 0) return hit;
		hit.t = (t2 > 0) ? t2 : t1;

		hit.position = ray.start + ray.dir * hit.t;
		if (hit.position.z > top || hit.position.z < bot) {
			Hit topHit;
			topHit.t = (ray.start.z - top) / (-ray.dir.z);
			Hit botHit;
			botHit.t = (ray.start.z - bot) / (-ray.dir.z);
			topHit.position = ray.start + ray.dir * topHit.t;
			if (topHit.t < botHit.t && dot(toVec4(topHit.position, 1.0f) * Q, toVec4(topHit.position, 1.0f)) < 0) {
				topHit.material = material;
				topHit.normal = vec3(0, 0, 1);
				return topHit;
			}
			
			botHit.position = ray.start + ray.dir * botHit.t;
			if (topHit.t > botHit.t && dot(toVec4(botHit.position, 1.0f) * Q, toVec4(botHit.position, 1.0f)) < 0) {
				botHit.material = material;
				botHit.normal = vec3(0, 0, -1);
				return botHit;
			}
			return Hit();
		}
		else {
			vec4 gradf = toVec4(hit.position * 2.0f, 1.0) * Q;
			hit.normal = normalize(vec3(gradf.x, gradf.y, gradf.z));
			hit.material = material;
			return hit;
		}
	}
};

struct Hiberboloid : public Intersectable {
	mat4 Q;
	float a, b, c;
	vec3 center;
	float bot = -0.6f;
	float top = -0.3f;

	Hiberboloid(const vec3& _center, float _a, float _b, float _c, float _bot, float _top, Material* _material) {
		center = _center;
		a = _a;
		b = _b;
		c = _c;
		bot = _bot;
		top = _top;
		float x = (-2.0f * _center.x) / (a * a);
		float x2 = _center.x * _center.x / (a * a);
		float y = (-2.0f * _center.y) / (b * b);
		float y2 = _center.y * _center.y / (b * b);
		float z = (-2.0f * _center.y) / (c * c);
		float z2 = _center.y * _center.y / (c * c);
		Q = mat4(
			1.0f / (a * a), 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f / (b * b), 0.0f, 0.0f,
			0.0f, 0.0f, -1.0f / (c * c), 0.0f,
			x, y, -z, -1.0f + x2 + y2 - z2
		);
		material = _material;
	}
	Hit intersect(const Ray& ray) {
		Hit hit;
		vec4 rayDir = toVec4(ray.dir, 0);
		vec4 rayStart = toVec4(ray.start, 1);
		float a = dot(rayDir * Q, rayDir);
		float b = dot(rayDir * Q, rayStart) + dot(rayStart * Q, rayDir);
		float c = dot(rayStart * Q, rayStart);

		float discr = b * b - 4.0f * a * c;
		if (discr < 0) return hit;
		float sqrt_discr = sqrtf(discr);
		float t1 = (-b + sqrt_discr) / 2.0f / a;	// t1 >= t2 for sure
		float t2 = (-b - sqrt_discr) / 2.0f / a;
		if (t1 <= 0) return hit;
		hit.t = (t2 > 0) ? t2 : t1;

		hit.position = ray.start + ray.dir * hit.t;
		if (hit.position.z > top || hit.position.z < bot) {
			Hit topHit;
			Hit botHit;
			topHit.t = (ray.start.z - top) / (-ray.dir.z);
			botHit.t = (ray.start.z - bot) / (-ray.dir.z);
			topHit.position = ray.start + ray.dir * topHit.t;
			if (topHit.t < botHit.t && dot(toVec4(topHit.position, 1.0f) * Q, toVec4(topHit.position, 1.0f)) < 0) {
				topHit.material = material;
				topHit.normal = vec3(0, 0, 1);
				return topHit;
			}
			
			
			botHit.position = ray.start + ray.dir * botHit.t;
			if (topHit.t > botHit.t && dot(toVec4(botHit.position, 1.0f) * Q, toVec4(botHit.position, 1.0f)) < 0) {
				botHit.material = material;
				botHit.normal = vec3(0, 0, -1);
				return botHit;
			}
			return Hit();
		}
		else {
			vec4 gradf = toVec4(hit.position * 2.0f, 1.0) * Q;
			hit.normal = normalize(vec3(gradf.x, gradf.y, gradf.z));
			hit.material = material;
			return hit;
		}
	}
};

struct Hole : Hiberboloid {
	float radius;
	Hole() :
		Hiberboloid(
			vec3(0,YPOS, Z),
			A, A, C,
			BOT, TOP,// Magic numbers
			new ReflectiveMaterial(vec3(0.14, 0.16, 0.13), vec3(4.1, 2.3, 3.1))
		)
	{}
	Hit intersect(const Ray& ray) {
		Hit hit;
		vec4 rayDir = toVec4(ray.dir, 0);
		vec4 rayStart = toVec4(ray.start, 1);
		float a = dot(rayDir * Q, rayDir);
		float b = dot(rayDir * Q, rayStart) + dot(rayStart * Q, rayDir);
		float c = dot(rayStart * Q, rayStart);

		float discr = b * b - 4.0f * a * c;
		if (discr < 0) return hit;
		float sqrt_discr = sqrtf(discr);
		float t1 = (-b + sqrt_discr) / 2.0f / a;	// t1 >= t2 for sure
		float t2 = (-b - sqrt_discr) / 2.0f / a;

		vec3 t1HitPos = ray.start + ray.dir * t1;
		vec3 t2HitPos = ray.start + ray.dir * t2;

		if (t1 <= 0) return hit;
		if (t2 > 0 && (t2HitPos.z < bot || t2HitPos.z > top)) {
			if (t1 > 0 && (t1HitPos.z < bot || t1HitPos.z > top)) {
				return Hit();
			}
			else if (t1 > 0 && t1HitPos.z > bot && t1HitPos.z<top)
				hit.t = t1;
		}
		else if( t2 > 0 && t2HitPos.z > bot && t2HitPos.z < top)
			hit.t = t2;
		else
			return Hit();

		hit.position = ray.start + ray.dir * hit.t;

		vec4 gradf = toVec4(hit.position * 2.0f, 1.0) * Q;
		hit.normal = normalize(vec3(gradf.x, gradf.y, gradf.z));
		hit.material = material;
		return hit;
	}
	bool inside(const vec3& pos) {
		if (pos.z < BOT)
			return false;
		return dot(toVec4(pos, 1.0f) * Q, toVec4(pos, 1.0f)) < 0;
	}
};

struct Room : Elipsoid {
	Hole* hole;
	Room(Hole* h, Material* material)
		:Elipsoid(vec3(0.0, 0.0, 0.0), 2.0f, 2.1f,1.0f, material){
		hole = h;
	}
	Hit intersect(const Ray& ray) {
		Hit hit = Elipsoid::intersect(ray);
		if (hole->inside(hit.position)) {
			hit.t = -1;
		}
		return hit;
	}
};
float rnd() { return (float)rand() / RAND_MAX; }
class Camera {
	vec3 eye, lookat, right, up;
public:
	void set(vec3 _eye, vec3 _lookat, vec3 vup, float fov) {
		eye = _eye;
		lookat = _lookat;
		vec3 w = eye - lookat;
		float focus = length(w);
		right = normalize(cross(vup, w)) * focus * tanf(fov / 2);
		up = normalize(cross(w, right)) * focus * tanf(fov / 2);
	}
	Ray getRay(int X, int Y) {
		vec3 dir = lookat + right * (2.0f * (X + 0.5f) / windowWidth - 1) + up * (2.0f * (Y + 0.5f) / windowHeight - 1) - eye;
		return Ray(eye, dir);
	}
};

struct Light {
	vec3 direction;
	vec3 Le;
	Light(vec3 _direction, vec3 _Le) {
		direction = normalize(_direction);
		Le = _Le;
	}
};



const float epsilon = 0.0001f;

class Scene {
	std::vector<Intersectable*> objects;
	std::vector<Light*> lights;
	std::vector<vec3> points;
	Camera camera;
	vec3 La; // �llatl�nos f�nyess�g
	vec3 skyLe;
	vec3 sunLe;
	vec3 sunDir;
public:
	void build() {
		vec3 eye = vec3(0, 2, 0), vup = vec3(0, 0, 1), lookat = vec3(0, 0, 0);
		float fov = 80 * M_PI / 180;
		camera.set(eye, lookat, vup, fov);

		La = vec3(0.1f, 0.1f, 0.1f);
	
		skyLe = vec3(20, 20, 20);
		sunLe = vec3(2, 2, 2);
		sunDir = normalize(vec3(0.3, 0.3, 1));
		
		vec3 kdCylinder(0.4f, 0.4f, 0.6f), ksCylinder(1, 1, 2);
		Material* cylinderMaterial = new RoughMaterial(kdCylinder, ksCylinder, 30);
		objects.push_back(new Cylinder(vec3(0.1, 0.4, 0.0), 0.2,0.2, -1.5, -0.3, cylinderMaterial));

		vec3 kdEll(0.4f, 0.6f, 0.4f), ksEll(1, 3, 2);
		Material* ellMaterial = new RoughMaterial(kdEll, ksEll, 50);
		objects.push_back(new Elipsoid(vec3(1, 0.1, 0), 0.6,0.9,0.4, ellMaterial));

		vec3 n (0.17f ,0.35f, 1.5f), kappa(3.1f, 2.7f, 1.9f);
		Material* material3 = new ReflectiveMaterial(n, kappa);
		objects.push_back(new Hiberboloid(vec3(-0.6, 0.1, 0.0),0.1,0.1,0.3, -1.5, 1.0, material3));
		//objects.push_back(new Elipsoid(vec3(0.0, -0.7, 0.0),0.3,0.2,0.7, material3));

		Hole* hole = new Hole();
		objects.push_back(hole);

		vec3 kd(0.4f, 0.3f, 0.3f), ks(3, 3, 3);
		Material* roomMaterial = new RoughMaterial(kd, ks, 50);
		objects.push_back(new Room(hole, roomMaterial));

	}

	void render(std::vector<vec4>& image) {
		for (int Y = 0; Y < windowHeight; Y++) {
#pragma omp parallel for
			for (int X = 0; X < windowWidth; X++) {
				vec3 color = trace(camera.getRay(X, Y));
				image[Y * windowWidth + X] = vec4(color.x, color.y, color.z, 1);
			}
		}
	}

	Hit firstIntersect(Ray ray) {
		Hit bestHit;
		for (Intersectable* object : objects) {
			Hit hit = object->intersect(ray); //  hit.t < 0 if no intersection
			if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t))  bestHit = hit;
		}
		if (dot(ray.dir, bestHit.normal) > 0) bestHit.normal = bestHit.normal * (-1);
		return bestHit;
	}

	bool shadowIntersect(Ray ray) {	// for directional lights
		for (Intersectable* object : objects) if (object->intersect(ray).t > 0) return true;
		return false;
	}

	vec3 trace(Ray ray, int roughDepth = 0, int reflectionDepth = 0) {
		if (reflectionDepth > 4)
			return {0,0,0};
		Hit hit = firstIntersect(ray);
		if (hit.t < 0) {
			return skyLe + sunLe * pow(dot(ray.dir, sunDir), 10);
		}
		vec3 outRadiance(0, 0, 0);
		if (hit.material->type == ROUGH && roughDepth == 1)
			return La;

		if (hit.material->type == ROUGH) {

			outRadiance = hit.material->ka * La;
			
			points.clear();
			while (points.size() != RESOLUTION) {
				float x = rnd() * RAD;
				float y = rnd() * RAD;
				if (x * x + y * y < RAD_SQUARE)
					points.push_back(vec3(x, y, BOT));
			}

			for (vec3 holePoint : points) {
				vec3 rayDir = normalize(holePoint - hit.position);
				float cosTheta = dot(hit.normal, rayDir);
				if (cosTheta > 0) {
					Ray newRay(hit.position + hit.normal * epsilon, rayDir);
					float dist2 = dot(holePoint - hit.position, holePoint - hit.position);
					float deltaOmega = deltaAREA * dot(rayDir, vec3(0, 0, 1)) / dist2;
					vec3 lightTraced = trace(newRay, roughDepth + 1, reflectionDepth) * deltaOmega;

					outRadiance = outRadiance + lightTraced * hit.material->kd * cosTheta;

					vec3 halfway = normalize(newRay.dir + -ray.dir);
					float cosDelta = dot(hit.normal, halfway);

					if (cosDelta > 0) outRadiance = outRadiance + lightTraced  * hit.material->ks * powf(cosDelta, hit.material->shininess);
				}
			}
		}
		if (hit.material->type == REFLECTIVE) {
			vec3 reflectedDir = ray.dir - hit.normal * dot(hit.normal, ray.dir) * 2.0f;
			float cosa = -dot(ray.dir, hit.normal);
			vec3 one(1, 1, 1);
			vec3 F = hit.material->F0 + (one - hit.material->F0) * pow(1 - cosa, 5);
			outRadiance = outRadiance + trace(Ray(hit.position + hit.normal * epsilon, reflectedDir), roughDepth, reflectionDepth + 1) * F;
		}
		return outRadiance;
	}
};

GPUProgram gpuProgram; // vertex and fragment shaders
Scene scene;

// vertex shader in GLSL
const char* vertexSource = R"(
	#version 330
    precision highp float;

	layout(location = 0) in vec2 cVertexPosition;	// Attrib Array 0
	out vec2 texcoord;

	void main() {
		texcoord = (cVertexPosition + vec2(1, 1))/2;							// -1,1 to 0,1
		gl_Position = vec4(cVertexPosition.x, cVertexPosition.y, 0, 1); 		// transform to clipping space
	}
)";

// fragment shader in GLSL
const char* fragmentSource = R"(
	#version 330
    precision highp float;

	uniform sampler2D textureUnit;
	in  vec2 texcoord;			// interpolated texture coordinates
	out vec4 fragmentColor;		// output that goes to the raster memory as told by glBindFragDataLocation

	void main() {
		fragmentColor = texture(textureUnit, texcoord); 
	}
)";

class FullScreenTexturedQuad {
	unsigned int vao;	// vertex array object id and texture id
	Texture texture;
public:
	FullScreenTexturedQuad(int windowWidth, int windowHeight, std::vector<vec4>& image)
		: texture(windowWidth, windowHeight, image)
	{
		glGenVertexArrays(1, &vao);	// create 1 vertex array object
		glBindVertexArray(vao);		// make it active

		unsigned int vbo;		// vertex buffer objects
		glGenBuffers(1, &vbo);	// Generate 1 vertex buffer objects

		// vertex coordinates: vbo0 -> Attrib Array 0 -> vertexPosition of the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, vbo); // make it active, it is an array
		float vertexCoords[] = { -1, -1,  1, -1,  1, 1,  -1, 1 };	// two triangles forming a quad
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);	   // copy to that part of the memory which is not modified 
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);     // stride and offset: it is tightly packed
	}

	void Draw() {
		glBindVertexArray(vao);	// make the vao and its vbos active playing the role of the data source
		gpuProgram.setUniform(texture, "textureUnit");
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);	// draw two triangles forming a quad
	}
};

FullScreenTexturedQuad* fullScreenTexturedQuad;

// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	scene.build();

	std::vector<vec4> image(windowWidth * windowHeight);
	long timeStart = glutGet(GLUT_ELAPSED_TIME);
	scene.render(image);
	long timeEnd = glutGet(GLUT_ELAPSED_TIME);
	printf("Rendering time: %d milliseconds\n", (timeEnd - timeStart));

	// copy image to GPU as a texture
	fullScreenTexturedQuad = new FullScreenTexturedQuad(windowWidth, windowHeight, image);

	// create program for the GPU
	gpuProgram.create(vertexSource, fragmentSource, "fragmentColor");
}

// Window has become invalid: Redraw
void onDisplay() {
	fullScreenTexturedQuad->Draw();
	glutSwapBuffers();									// exchange the two buffers
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {

}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
}
