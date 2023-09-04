#pragma once

#include "Utilities.h"
#include "WavefrontObjParser.h"


struct OpenGLUtils
{
    struct Vertex
    {
        float position[3];
        float normal[3];
        float colour[4];
        float texCoord[2];
    };

    struct Attributes
    {
        explicit Attributes (juce::OpenGLShaderProgram& shader)
        {
            position      .reset (createAttribute (shader, "position"));
            normal        .reset (createAttribute (shader, "normal"));
            sourceColour  .reset (createAttribute (shader, "sourceColour"));
            textureCoordIn.reset (createAttribute (shader, "textureCoordIn"));
        }

        void enable()
        {
            using namespace ::juce::gl;

            if (position.get() != nullptr)
            {
                glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), nullptr);
                glEnableVertexAttribArray (position->attributeID);
            }

            if (normal.get() != nullptr)
            {
                glVertexAttribPointer (normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 3));
                glEnableVertexAttribArray (normal->attributeID);
            }

            if (sourceColour.get() != nullptr)
            {
                glVertexAttribPointer (sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 6));
                glEnableVertexAttribArray (sourceColour->attributeID);
            }

            if (textureCoordIn.get() != nullptr)
            {
                glVertexAttribPointer (textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 10));
                glEnableVertexAttribArray (textureCoordIn->attributeID);
            }
        }

        void disable()
        {
            using namespace ::juce::gl;

            if (position.get() != nullptr)        glDisableVertexAttribArray (position->attributeID);
            if (normal.get() != nullptr)          glDisableVertexAttribArray (normal->attributeID);
            if (sourceColour.get() != nullptr)    glDisableVertexAttribArray (sourceColour->attributeID);
            if (textureCoordIn.get() != nullptr)  glDisableVertexAttribArray (textureCoordIn->attributeID);
        }

        std::unique_ptr<juce::OpenGLShaderProgram::Attribute> position, normal, sourceColour, textureCoordIn;

    private:
        static juce::OpenGLShaderProgram::Attribute* createAttribute (juce::OpenGLShaderProgram& shader,
                                                                const char* attributeName)
        {
            using namespace ::juce::gl;

            if (glGetAttribLocation (shader.getProgramID(), attributeName) < 0)
                return nullptr;

            return new juce::OpenGLShaderProgram::Attribute (shader, attributeName);
        }
    };

    struct Uniforms
    {
        explicit Uniforms (juce::OpenGLShaderProgram& shader)
        {
            projectionMatrix.reset (createUniform (shader, "projectionMatrix"));
            viewMatrix      .reset (createUniform (shader, "viewMatrix"));
            texture         .reset (createUniform (shader, "demoTexture"));
            lightPosition   .reset (createUniform (shader, "lightPosition"));
            bouncingNumber  .reset (createUniform (shader, "bouncingNumber"));
            viewResolution  .reset (createUniform (shader, "viewResolution"));
            timeKeeper      .reset (createUniform (shader, "timeKeeper"));
            audioAmplitude  .reset (createUniform (shader, "audioAmplitude"));
            synthNoteColor  .reset (createUniform (shader, "synthNoteColor"));
        }

        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, texture, lightPosition, bouncingNumber, 
                                                      viewResolution, timeKeeper, audioAmplitude, synthNoteColor;

    private:
        static juce::OpenGLShaderProgram::Uniform* createUniform (juce::OpenGLShaderProgram& shader,
                                                            const char* uniformName)
        {
            using namespace ::juce::gl;

            if (glGetUniformLocation (shader.getProgramID(), uniformName) < 0)
                return nullptr;

            return new juce::OpenGLShaderProgram::Uniform (shader, uniformName);
        }
    };

    struct Shape
    {
        Shape()
        {
            juce::String crabString = loadEntireAssetIntoString("teapot.obj");
            if (shapeFile.load (crabString).wasOk())
                for (auto* s : shapeFile.shapes)
                    vertexBuffers.add (new VertexBuffer (*s));
        }

        void draw (Attributes& attributes)
        {
            using namespace ::juce::gl;

            for (auto* vertexBuffer : vertexBuffers)
            {
                vertexBuffer->bind();

                attributes.enable();
                glDrawElements (GL_TRIANGLES, vertexBuffer->numIndices, GL_UNSIGNED_INT, nullptr);
                attributes.disable();
            }
        }

    private:
        struct VertexBuffer
        {
            explicit VertexBuffer (WavefrontObjFile::Shape& shape)
            {
                using namespace ::juce::gl;

                numIndices = shape.mesh.indices.size();

                glGenBuffers (1, &vertexBuffer);
                glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);

                juce::Array<Vertex> vertices;
                createVertexListFromMesh (shape.mesh, vertices, juce::Colours::green);

                glBufferData (GL_ARRAY_BUFFER, vertices.size() * (int) sizeof (Vertex),
                              vertices.getRawDataPointer(), GL_STATIC_DRAW);

                glGenBuffers (1, &indexBuffer);
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                glBufferData (GL_ELEMENT_ARRAY_BUFFER, numIndices * (int) sizeof (juce::uint32),
                                                       shape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
            }

            ~VertexBuffer()
            {
                using namespace ::juce::gl;

                glDeleteBuffers (1, &vertexBuffer);
                glDeleteBuffers (1, &indexBuffer);
            }

            void bind()
            {
                using namespace ::juce::gl;

                glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            }

            GLuint vertexBuffer, indexBuffer;
            int numIndices;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
        };

        WavefrontObjFile shapeFile;
        juce::OwnedArray<VertexBuffer> vertexBuffers;

        static void createVertexListFromMesh (const WavefrontObjFile::Mesh& mesh, juce::Array<Vertex>& list, juce::Colour colour)
        {
            auto scale = 0.2f;
            WavefrontObjFile::TextureCoord defaultTexCoord = { 0.5f, 0.5f };
            WavefrontObjFile::Vertex defaultNormal = { 0.5f, 0.5f, 0.5f };

            for (int i = 0; i < mesh.vertices.size(); ++i)
            {
                auto& v = mesh.vertices.getReference (i);

                auto& n = (i < mesh.normals.size() ? mesh.normals.getReference (i)
                                                   : defaultNormal);

                auto& tc = (i < mesh.textureCoords.size() ? mesh.textureCoords.getReference (i)
                                                          : defaultTexCoord);

                list.add ({ { scale * v.x, scale * v.y, scale * v.z, },
                            { scale * n.x, scale * n.y, scale * n.z, },
                            { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
                            { tc.x, tc.y } });
            }
        }
    };

    struct ShaderPreset
    {
        const char* name;
        const char* vertexShader;
        const char* fragmentShader;
    };

    static juce::Array<ShaderPreset> getPresets()
    {
        #define SHADER_DEMO_HEADER \
            "/*  This is a live OpenGL Shader demo.\n" \
            "    Edit the shader program below and it will be \n" \
            "    compiled and applied to the model above!\n" \
            "*/\n\n"

        ShaderPreset presets[] =
        {
            {
                "Plasma Globe",

                SHADER_DEMO_HEADER
                "attribute vec2 position;\n"
                "varying vec2 textureCoordOut;\n\n"
                "void main()\n"
                "{\n"
                "textureCoordOut = position * 0.5 + 0.5;\n"
                "gl_Position = vec4(position, 0.0, 1.0);\n"
                "}\n",

                SHADER_DEMO_HEADER
                "uniform float timeKeeper;\n"
                "uniform vec2 viewResolution;\n"
                "uniform float audioAmplitude;\n"
                "uniform float synthNoteColor;\n\n"
                "#define ENABLE_MOUSE 1\n"
                "#define NUM_RAYS 10.\n"
                "#define VOLUMETRIC_STEPS 19\n"
                "#define MAX_ITER 35\n"
                "#define FAR 6.\n"
                "#define time timeKeeper*1.1\n"
                "\n"
                "vec3 palette(float t)\n"
                "{\n"
                "   vec3 a = vec3(0.6, 0.4, 0.5);\n"
                "   vec3 b = vec3(0.7, 0.3, 0.5);\n"
                "   vec3 c = vec3(0.8, 1.0, 0.6);\n"
                "   vec3 d = vec3(0.76, 0.41, 0.96);\n\n"
                "   return a + b * cos(6.28318 * (c * t + d));\n"
                "}\n"
                "mat2 mm2(in float a){float c = cos(a), s = sin(a);return mat2(c,-s,s,c);}\n"
                "float hash1D(float n) {\n"
                "   return fract(sin(n) * 43758.5453);\n"
                "}\n"
                "\n"
                "float noise(float x) {\n"
                "    float i = floor(x);\n"
                "    float f = fract(x);\n"
                "    float a = hash1D(i);\n"
                "    float b = hash1D(i + 1.0);\n"
                "    return mix(a, b, smoothstep(0.0, 1.0, f));\n"
                "}\n"
                "\n"
                "vec3 hash3D(vec3 p) {\n"
                "    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),\n"
                "             dot(p, vec3(269.5, 183.3, 246.1)),\n"
                "             dot(p, vec3(113.5, 271.9, 124.6)));\n"
                "    return -1.0 + 2.0 * fract(sin(p) * 43758.5453);\n"
                "}\n"
                "\n"
                "float noise(vec3 p) {\n"
                "    vec3 i = floor(p);\n"
                "    vec3 f = fract(p);\n"
                "    f = f * f * (3.0 - 2.0 * f);\n"
                "\n"
                "    vec3 a = hash3D(i);\n"
                "    vec3 b = hash3D(i + vec3(1.0, 0.0, 0.0));\n"
                "    vec3 c = hash3D(i + vec3(0.0, 1.0, 0.0));\n"
                "    vec3 d = hash3D(i + vec3(1.0, 1.0, 0.0));\n"
                "    vec3 e = hash3D(i + vec3(0.0, 0.0, 1.0));\n"
                "    vec3 f1 = hash3D(i + vec3(1.0, 0.0, 1.0));\n"
                "    vec3 g = hash3D(i + vec3(0.0, 1.0, 1.0));\n"
                "    vec3 h = hash3D(i + vec3(1.0, 1.0, 1.0));\n"
                "\n"
                "    float va = dot(a, f);\n"
                "    float vb = dot(b, f - vec3(1.0, 0.0, 0.0));\n"
                "    float vc = dot(c, f - vec3(0.0, 1.0, 0.0));\n"
                "    float vd = dot(d, f - vec3(1.0, 1.0, 0.0));\n"
                "    float ve = dot(e, f - vec3(0.0, 0.0, 1.0));\n"
                "    float vf = dot(f1, f - vec3(1.0, 0.0, 1.0));\n"
                "    float vg = dot(g, f - vec3(0.0, 1.0, 1.0));\n"
                "    float vh = dot(h, f - vec3(1.0, 1.0, 1.0));\n"
                "\n"
                "    return mix(mix(mix(va, vb, f.x), mix(vc, vd, f.x), f.y),\n"
                "               mix(mix(ve, vf, f.x), mix(vg, vh, f.x), f.y), f.z);\n"
                "}\n"
                "\n"
                "mat3 m3 = mat3( 0.00,  0.80,  0.60,\n"
                "               -0.80,  0.36, -0.48,\n"
                "               -0.60, -0.48,  0.64);\n"
                "\n"
                "float flow(in vec3 p, in float t)\n"
                "{\n"
                "   float z=2.;\n"
                "   float rz = 0.;\n"
                "   vec3 bp = p;\n"
                "   for (float i= 1.;i < 5.;i++ )\n"
                "   {\n"
                "       p += time*.1;\n"
                "       rz+= (sin(noise(p+t*0.8)*6.)*0.5+0.5) /z;\n"
                "       p = mix(bp,p,0.6);\n"
                "       z *= 2.;\n"
                "       p *= 2.01;\n"
                "       p*= m3;\n"
                "   }\n"
                "   return rz;\n"
                "}\n"
                "\n"
                "float sins(in float x)\n"
                "{\n"
                "   float rz = 0.;\n"
                "   float z = 2.;\n"
                "   for (float i= 0.;i < 3.;i++ )\n"
                "   {\n"
                "       rz += abs(fract(x*1.4)-0.5)/z;\n"
                "       x *= 1.3;\n"
                "       z *= 1.15;\n"
                "       x -= time*.65*z;\n"
                "   }\n"
                "   return rz;\n"
                "}\n"
                "\n"
                "float segm( vec3 p, vec3 a, vec3 b)\n"
                "{\n"
                "   vec3 pa = p - a;\n"
                "   vec3 ba = b - a;\n"
                "   float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1. );\n"
                "   return length( pa - ba*h )*.5;\n"
                "}\n"
                "\n"
                "vec3 path(in float i, in float d, vec3 hit)\n"
                "{\n"
                "   vec3 en = vec3(0.,0.,1.);\n"
                "   float sns2 = sins(d+i*0.5)*0.22;\n"
                "   float sns = sins(d+i*.6)*0.21;\n"
                "\n"
                "   if (dot(hit,hit)>0.) {\n"
                "       hit.xz *= mm2(sns2*.5);\n"
                "       hit.xy *= mm2(sns*.3);\n"
                "       return hit;\n"
                "   }\n"
                "\n"
                "   en.xz *= mm2((hash1D(i*10.569)-.5)*6.2+sns2);\n"
                "   en.xy *= mm2((hash1D(i*4.732)-.5)*6.2+sns);\n"
                "\n"
                "   return en;\n"
                "}\n"
                "\n"
                "vec2 map(vec3 p, float i, vec3 hit)\n"
                "{\n"
                "   vec3 p0 = p;\n"
                "   float lp = length(p);\n"
                "   vec3 bg = vec3(0.);\n"
                "   vec3 en = path(i,lp, hit);\n"
                "\n"
                "   float ins = smoothstep(0.11,.46,lp);\n"
                "   float outs = .15+smoothstep(.0,.15,abs(lp-1.));\n"
                "   p *= ins*outs;\n"
                "   float id = ins*outs;\n"
                "\n"
                "   float rz = segm(p, bg, en)-0.011;\n"
                "\n"
                "   return vec2(rz,id);\n"
                "}\n"
                "\n"
                "float march(in vec3 ro, in vec3 rd, in float startf, in float maxd, in float j, vec3 hit)\n"
                "{\n"
                "   float precis = 0.001;\n"
                "   float h=0.5;\n"
                "   float d = startf;\n"
                "   for( int i=0; i<MAX_ITER; i++ )\n"
                "   {\n"
                "       if( abs(h)<precis||d>maxd ) break;\n"
                "       d += h*1.2;\n"
                "       float res = map(ro+rd*d, j, hit).x;\n"
                "       h = res;\n"
                "   }\n"
                "   return d;\n"
                "}\n"
                "\n"
                "vec3 vmarch(in vec3 ro, in vec3 rd, in float j, in vec3 orig, vec3 hit)\n"
                "{\n"
                "   vec3 p = ro;\n"
                "   vec2 r = vec2(0.);\n"
                "   vec3 sum = vec3(0);\n"
                "   float w = 0.;\n"
                "   for( int i=0; i<VOLUMETRIC_STEPS; i++ )\n"
                "   {\n"
                "       r = map(p,j,hit);\n"
                "       p += rd*.03;\n"
                "       float lp = length(p);\n"
                "\n"
                "       vec3 col = sin(vec3(1.05,2.5,1.52)*3.94+r.y)*.85+0.4;\n"
                "       col.rgb *= smoothstep(.0,.015,-r.x);\n"
                "       col *= smoothstep(0.04,.2,abs(lp-1.1));\n"
                "       col *= smoothstep(0.1,.34,lp);\n"
                "       sum += abs(col)*5. * (1.2-noise(lp*2.+j*13.+time*5.)*1.1) / (log(distance(p,orig)-2.)+.75);\n"
                "   }\n"
                "   return sum;\n"
                "}\n"
                "\n"
                "vec2 iSphere2(in vec3 ro, in vec3 rd)\n"
                "{\n"
                "   vec3 oc = ro;\n"
                "   float b = dot(oc, rd);\n"
                "   float c = dot(oc,oc) - 1.;\n"
                "   float h = b*b - c;\n"
                "   if(h <0.0) return vec2(-1.);\n"
                "   else return vec2((-b - sqrt(h)), (-b + sqrt(h)));\n"
                "}\n"
                "\n"
                "void main()\n"
                "{\n"
                "   vec2 p = gl_FragCoord.xy/viewResolution.xy-0.5;\n"
                "   p.x*=viewResolution.x/viewResolution.y;\n"
                "\n"
                "   vec3 ro = vec3(0.,0.,5.);\n"
                "   vec3 rd = normalize(vec3(p*.7,-1.5));\n"
                "\n"
                "   mat2 mx = mm2(time*.4);\n"
                "   mat2 my = mm2(time*0.3);\n"
                "   ro.xz *= mx;rd.xz *= mx;\n"
                "   ro.xy *= my;rd.xy *= my;\n"
                "\n"
                "   vec3 bro = ro;\n"
                "   vec3 brd = rd;\n"
                "\n"
                "   vec3 col = vec3(0.0125,0.,0.025) * palette(synthNoteColor);\n"
                "   #if 1\n"
                "   for (float j = 1.;j<NUM_RAYS+1.;j++)\n"
                "   {\n"
                "       ro = bro;\n"
                "       rd = brd;\n"
                "       mat2 mm = mm2((time*0.1+((j+1.)*5.1))*j*0.25);\n"
                "\n"
                "       float rz = march(ro,rd,2.5,FAR,j, vec3(0.));\n"
                "       if ( rz >= FAR)continue;\n"
                "       vec3 pos = ro+rz*rd;\n"
                "       col = max(col,vmarch(pos,rd,j, bro, vec3(0.))) * palette(synthNoteColor);\n"
                "   }\n"
                "   #endif\n"
                "\n"
                "   #if ENABLE_MOUSE\n"
                "   vec3 hit = vec3(0.);\n"
                "\n"
                "\n"
                "   if (dot(hit, hit) != 0.)\n"
                "   {\n"
                "       float j = NUM_RAYS+1.;\n"
                "       ro = bro;\n"
                "       rd = brd;\n"
                "       mat2 mm = mm2((time*0.1+((j+1.)*5.1))*j*0.25);\n"
                "\n"
                "      float rz = march(ro,rd,2.5,FAR,j, hit);\n"
                "       if ( rz < FAR) {\n"
                "           vec3 pos = ro+rz*rd;\n"
                "           col = max(col,vmarch(pos,rd,j, bro, hit));\n"
                "       }\n"
                "   }\n"
                "   #endif\n"
                "\n"
                "   ro = bro;\n"
                "   rd = brd;\n"
                "   vec2 sph = iSphere2(ro,rd);\n"
                "\n"
                "   if (sph.x > 0.)\n"
                "   {\n"
                "       vec3 pos = ro+rd*sph.x;\n"
                "       vec3 pos2 = ro+rd*sph.y;\n"
                "       vec3 rf = reflect( rd, pos );\n"
                "       vec3 rf2 = reflect( rd, pos2 );\n"
                "       float nz = (-log(abs(flow(rf*1.2,time)-.01)));\n"
                "       float nz2 = (-log(abs(flow(rf2*1.2,-time)-.01)));\n"
                "       col += (0.1*nz*nz* vec3(0.12,0.12,.5) + 0.05*nz2*nz2*vec3(0.55,0.2,.55))*0.6 + 0.2 * audioAmplitude;\n"
                "   }\n"
                "   gl_FragColor = vec4(col*1.3, 1.0);\n"
                "}\n"
            },
            {
                "Fractal Pyramid",

                SHADER_DEMO_HEADER
                "attribute vec2 position;\n\n"
                "varying vec2 textureCoordOut;\n\n"
                "void main()\n"
                "{\n"
                "textureCoordOut = position * 0.5 + 0.5;\n"
                "gl_Position = vec4(position, 0.0, 1.0);\n"
                "}\n",

                SHADER_DEMO_HEADER
                "uniform float timeKeeper;\n"
                "uniform vec2 viewResolution;\n"
                "uniform float audioAmplitude;\n"
                "uniform float synthNoteColor;\n\n"
                "vec3 palette(float t)\n"
                "{\n"
                "   vec3 a = vec3(0.6, 0.4, 0.5);\n"
                "   vec3 b = vec3(0.7, 0.3, 0.5);\n"
                "   vec3 c = vec3(0.8, 1.0, 0.6);\n"
                "   vec3 d = vec3(0.76, 0.41, 0.96);\n\n"
                "   return a + b * cos(6.28318 * (c * t + d));\n"
                "}\n"
                "vec2 rotate(vec2 p,float a)\n"
                "{\n"
                "   float c = cos(a);\n"
                "   float s = sin(a);\n"
                "   return p * mat2(c,s,-s,c);\n"
                "}\n"
                "\n"
                "float map(vec3 p)\n"
                "{\n"
                "   for (int i = 0; i < 8; ++i)\n"
                "   {\n"
                "       float t = timeKeeper * 0.2;\n"
                "       p.xz = rotate(p.xz,t);\n"
                "       p.xy = rotate(p.xy,t * 1.89);\n"
                "       p.xz = abs(p.xz);\n"
                "       p.xz -= .5;\n"
                "   }\n"
                "   return dot(sign(p),p) / 5.;\n"
                "}\n"
                "\n"
                "vec4 rm(vec3 ro, vec3 rd)\n"
                "{\n"
                "   float t = 0.;\n"
                "   vec3 col = vec3(0.);\n"
                "   float d;\n"
                "   for (float i = 0.; i < 64.; i++)\n"
                "   {\n"
                "       vec3 p = ro + rd * t;\n"
                "       d = map(p) * .5;\n"
                "       if (d < 0.02)\n"
                "           break;\n"
                "       if (d > 100.)\n"
                "           break;\n"

                "       col += palette(synthNoteColor + timeKeeper * 0.4) / (400. * (d));\n"
                "       t += d;\n"
                "   }\n"
                "   return vec4(col,1. / (d * 100.));\n"
                "}\n"
                "void main()\n"
                "{\n"
                "    vec2 uv = (gl_FragCoord.xy - (viewResolution.xy / 2.)) / viewResolution.x;\n"
                "    vec3 ro = vec3(0.,0.,-50.);\n"
                "    ro.xz = rotate(ro.xz,timeKeeper);\n"
                "    vec3 cf = normalize(-ro);\n"
                "    vec3 cs = normalize(cross(cf,vec3(0.,1.,0.)));\n"
                "    vec3 cu = normalize(cross(cf,cs));\n"
                "    vec3 uuv = ro + cf * 3. + uv.x * cs + uv.y * cu;\n"
                "    vec3 rd = normalize(uuv - ro);\n"
                "    vec4 col = rm(ro,rd) + audioAmplitude;\n"
                "    gl_FragColor = col; \n"
                "}\n"
            },
            {
                "Fractal Kaleidoscope",

                SHADER_DEMO_HEADER
                "attribute vec2 position;\n"
                "varying vec2 textureCoordOut;\n"
                "\n"
                "void main()\n"
                "{\n"
                "textureCoordOut = position * 0.5 + 0.5;\n"
                "gl_Position = vec4(position, 0.0, 1.0);\n"
                "}\n",

                SHADER_DEMO_HEADER
                "uniform float timeKeeper;\n"
                "uniform vec2 viewResolution;\n"
                "uniform float audioAmplitude;\n"
                "uniform float synthNoteColor;\n\n"
                "vec3 palette(float t)\n"
                "{\n"
                "   vec3 a = vec3(0.6, 0.4, 0.5);\n"
                "   vec3 b = vec3(0.7, 0.3, 0.5);\n"
                "   vec3 c = vec3(0.8, 1.0, 0.6);\n"
                "   vec3 d = vec3(0.76, 0.41, 0.96);\n\n"
                "   return a + b * cos(6.28318 * (c * t + d));\n"
                "}\n"
                "\n"
                "void main()\n"
                "{\n"
                "   vec2 uv = (gl_FragCoord.xy * 2.0 - viewResolution.xy) / viewResolution.y;\n"
                "   vec2 uv0 = uv;\n"
                "   vec3 finalColor = vec3(0.0);\n"
                "   for (float i = 0.0; i < 4.0; ++i)\n"
                "   {\n"
                "       uv = fract(uv * 1.6) - 0.5;\n"
                "       float d = length(uv) * exp(-length(uv0));\n"
                "       vec3 color = palette(length(uv0) + i * 0.2 + timeKeeper * 0.4 + synthNoteColor);\n"
                "       d = abs(sin(d*12.0 + timeKeeper + audioAmplitude) / 12.0);\n"
                "       d = pow(0.015 / d, 1.3);\n"
                "       finalColor += (color / 4.0) * d;\n"
                "   }\n"
                "    gl_FragColor = vec4(finalColor, 1.0);\n"
                "}\n"
            },
            {
                "Liquid Ball",

                SHADER_DEMO_HEADER
                "attribute vec2 position;\n"
                "varying vec2 textureCoordOut;\n\n"
                "void main()\n"
                "{\n"
                "textureCoordOut = position * 0.5 + 0.5;\n"
                "gl_Position = vec4(position, 0.0, 1.0);\n"
                "}\n",

                SHADER_DEMO_HEADER
                "uniform float timeKeeper;\n"
                "uniform vec2 viewResolution;\n"
                "uniform float audioAmplitude;\n"
                "uniform float synthNoteColor;\n\n"
                "#define INNER_RADIUS 0.75\n"
                "#define OUTER_RADIUS 0.9\n"
                "#define SHEET_THICKNESS 0.012\n"
                "#define NOISINESS 10.0\n"
                "#define INNER_COLOR vec4(0.0, 30.0, 30.0, 1.0)\n"
                "#define OUTER_COLOR vec4(20.0, 20.0, 30.0, 1.0)\n"
                "#define NUM_STEPS 20\n"
                "#define TIME_SCALE 5.0\n\n"
                "vec3 palette(float t)\n"
                "{\n"
                "   vec3 a = vec3(0.6, 0.4, 0.5);\n"
                "   vec3 b = vec3(0.7, 0.3, 0.5);\n"
                "   vec3 c = vec3(0.8, 1.0, 0.6);\n"
                "   vec3 d = vec3(0.76, 0.41, 0.96);\n\n"
                "   return a + b * cos(6.28318 * (c * t + d));\n"
                "}\n"
                "\n"
                "float trapezium(float x)\n"
                "{\n"
                "   return min(1.0, max(0.0, 1.0 - abs(-mod(x, 1.0) * 3.0 + 1.0)) * 2.0);\n"
                "}\n"

                "vec3 colFromHue(float hue)\n"
                "{\n"
                    "return vec3(trapezium(hue - 1.0 / 3.0), trapezium(hue), trapezium(hue + 1.0 / 3.0)) * palette(synthNoteColor);\n"
                "}\n\n"
                "float cnoise3(float pos)\n"
                "{\n"
                "   return (cos(pos / 2.0) * 0.2 + 1.0);\n"
                "}\n\n"
                "float cnoise2(float pos)\n"
                "{\n"
                "    return (sin(pos * cnoise3(pos) / 2.0) * 0.2 + 1.0);\n"
                "}\n\n"
                "float cnoise(vec4 pos)\n"
                "{\n"
                "   float x = pos.x * cnoise2(pos.y) + pos.w * 0.87123 + 82.52;\n"
                "   float y = pos.y * cnoise2(pos.z) + pos.w * 0.78725 + 12.76;\n"
                "   float z = pos.z * cnoise2(pos.x) + pos.w * 0.68201 + 42.03;\n"
                "   return (sin(x) + sin(y) + sin(z)) / 3.0;\n"
                "}\n\n"
                "vec4 merge_colours(vec4 apply_this, vec4 on_top_of_this)\n"
                "{\n"
                "    return on_top_of_this * (1.0 - apply_this.a) + apply_this * apply_this.a;\n"
                "}\n\n"
                "vec4 getdensity(vec3 pos)\n"
                "{\n"
                "    float time = timeKeeper * TIME_SCALE;\n"
                "    vec3 samplePos = normalize(pos);\n"
                "    vec4 inner_color = vec4(colFromHue(cnoise(vec4(samplePos / 5.0, time / 15.0))) * 25.0, 0.5 + (audioAmplitude * 0.5));\n"
                "    inner_color = merge_colours(inner_color, vec4(palette(synthNoteColor), 1.0));\n"
                "    vec4 outer_color = merge_colours(vec4(25.0,25.0,25.0,0.5), inner_color);\n"
                "    float sample_ = (cnoise(vec4(samplePos * NOISINESS, time)) + 1.0) / 2.0;\n"
                "    sample_ = clamp(sample_, 0.0, 1.0);\n"
                "    float innerIncBorder = INNER_RADIUS + SHEET_THICKNESS;\n"
                "    float outerIncBorder = OUTER_RADIUS - SHEET_THICKNESS;\n"
                "    float radius = innerIncBorder + (outerIncBorder - innerIncBorder) * sample_;\n"
                "    float dist = distance(pos, vec3(0.0, 0.0, 0.0));\n"
                "    float density = exp(-pow(dist - radius, 2.0) * 05000.0) * 0.7 + (audioAmplitude * 0.3);\n"
                "    return (inner_color + (outer_color - inner_color) * (radius - innerIncBorder) / (outerIncBorder - innerIncBorder)) * density;\n"
                "}\n\n"
                "vec4 raymarch(vec3 start, vec3 end)\n"
                "{\n"
                "    vec4 retn = vec4(0.0, 0.0, 0.0, 0.0);\n"
                "    vec3 delta = end - start;\n"
                "    float stepDistance = length(delta) / float(NUM_STEPS);\n"

                "    vec4 densityPrevious = getdensity(start);\n"
                "    for (int i = 1; i < NUM_STEPS; i++)\n"
                "    {\n"
                "        vec3 samplePos = start + delta * float(i) / float(NUM_STEPS);\n"
                "        vec4 density = getdensity(samplePos);\n"
                "        vec4 densityIntegrated = (density + densityPrevious) / 2.0;\n"
                "        retn += densityIntegrated;\n"
                "        densityPrevious = density;\n"
                "    }\n"
                "    return retn * stepDistance;\n"
                "}\n\n"
                "vec4 raymarch_ball(vec2 coord)\n"
                "{\n"
                "    float d = distance(coord, vec2(0.0, 0.0));\n"
                "    if (d > OUTER_RADIUS) {\n"
                "        return vec4(0.0, 0.0, 0.0, 0.0);\n"
                "    }\n"
                "    float dOuterNormalized = d / OUTER_RADIUS;\n"
                "    float outerStartZ = -sqrt(1.0 - dOuterNormalized * dOuterNormalized) * OUTER_RADIUS;\n"
                "    float outerEndZ = -outerStartZ;\n"
                "    if (d > INNER_RADIUS) {\n"
                "        vec4 frontPart = raymarch(vec3(coord, outerStartZ), vec3(coord, 0));\n"
                "        vec4 backPart = raymarch(vec3(coord, 0), vec3(coord, outerEndZ));\n"
                "        return frontPart + backPart;\n"
                "    }\n"

                "    float dInnerNormalized = d / INNER_RADIUS;\n"
                "    float innerStartZ = -sqrt(1.0 - dInnerNormalized * dInnerNormalized) * INNER_RADIUS;\n"
                "    float innerEndZ = -innerStartZ;\n"
                "    vec4 frontPart = raymarch(vec3(coord, outerStartZ), vec3(coord, innerStartZ));\n"
                "    vec4 backPart = raymarch(vec3(coord, innerEndZ), vec3(coord, outerEndZ));\n"
                "    vec4 final = frontPart + backPart;\n"
                "    return final;\n"
                "}\n\n"
                "void main()\n"
                "{\n"
                "    vec2 uv = (gl_FragCoord.xy / min(viewResolution.x, viewResolution.y)) * 2.0 - vec2(viewResolution.x / viewResolution.y, 1.0);\n"
                "    gl_FragColor = merge_colours(raymarch_ball(uv), vec4(0.0, 0.0, 0.0, 1.0));\n"
                "}\n"
            },
            {
                "Raymarching Tubes",

                SHADER_DEMO_HEADER
                "attribute vec2 position;\n"
                "varying vec2 textureCoordOut;\n\n"
                "void main()\n"
                "{\n"
                "textureCoordOut = position * 0.5 + 0.5;\n"
                "gl_Position = vec4(position, 0.0, 1.0);\n"
                "}\n",

                SHADER_DEMO_HEADER
                "uniform float timeKeeper;\n"
                "uniform vec2 viewResolution;\n"
                "uniform float audioAmplitude;\n"
                "uniform float synthNoteColor;\n\n"
                "vec3 palette(float t)\n"
                "{\n"
                "   vec3 a = vec3(0.6, 0.4, 0.5);\n"
                "   vec3 b = vec3(0.7, 0.3, 0.5);\n"
                "   vec3 c = vec3(0.8, 1.0, 0.6);\n"
                "   vec3 d = vec3(0.76, 0.41, 0.96);\n\n"
                "   return a + b * cos(6.28318 * (c * t + d));\n"
                "}\n"
                "float map(vec3 p) {\n"
                "   vec3 n = vec3(0, 1, 0);\n"
                "   float k1 = 1.9;\n"
                "   float k2 = (sin(p.x * k1) + sin(p.z * k1)) * 0.8;\n"
                "   float k3 = (sin(p.y * k1) + sin(p.z * k1)) * 0.8;\n"
                "   float w1 = 4.0 - dot(abs(p), normalize(n)) + k2;\n"
                "   float w2 = 4.0 - dot(abs(p), normalize(n.yzx)) + k3;\n"
                "   float s1 = length(mod(p.xy + vec2(sin((p.z + p.x) * 2.0) * 0.3, cos((p.z + p.x) * 1.0) * 0.5), 2.0) - 1.0) - 0.2;\n"
                "   float s2 = length(mod(0.5+p.yz + vec2(sin((p.z + p.x) * 2.0) * 0.3, cos((p.z + p.x) * 1.0) * 0.3), 2.0) - 1.0) - 0.2;\n"
                "   return min(w1, min(w2, min(s1, s2)));\n"
                "}\n"
                "\n"
                "vec2 rot(vec2 p, float a) {\n"
                "   return vec2(\n"
                "       p.x * cos(a) - p.y * sin(a),\n"
                "       p.x * sin(a) + p.y * cos(a));\n"
                "}\n"
                "\n"
                "void main() {\n"
                "   float time = timeKeeper;\n"
                "   vec2 uv = ( gl_FragCoord.xy / viewResolution.xy ) * 2.0 - 1.0;\n"
                "   uv.x *= viewResolution.x /  viewResolution.y;\n"
                "   vec3 dir = normalize(vec3(uv, 1.0));\n"
                "   dir.xz = rot(dir.xz, time * 0.23);dir = dir.yzx;\n"
                "   dir.xz = rot(dir.xz, time * 0.2);dir = dir.yzx;\n"
                "   vec3 pos = vec3(0, 0, time);\n"
                "   vec3 col = vec3(0.0);\n"
                "   float t = 0.0;\n"
                "    float tt = 0.0;\n"
                "   for(int i = 0 ; i < 100; i++) {\n"
                "       tt = map(pos + dir * t);\n"
                "       if(tt < 0.001) break;\n"
                "       t += tt * 0.45;\n"
                "   }\n"
                "   vec3 ip = pos + dir * t;\n"
                "   col = vec3(t * 0.2);\n"
                "   col = sqrt(col) * palette(synthNoteColor);\n"
                "   gl_FragColor = vec4(0.05*t+abs(dir) * col + max(0.0, map(ip - 0.1) - tt), 1.0);\n"
                "    gl_FragColor.a = 1.0 / (t * t * t * t * t);\n"
                "}\n"
            },
            {
                "The Universe",

                SHADER_DEMO_HEADER
                "attribute vec2 position;\n"
                "varying vec2 textureCoordOut;\n\n"
                "void main()\n"
                "{\n"
                "textureCoordOut = position * 0.5 + 0.5;\n"
                "gl_Position = vec4(position, 0.0, 1.0);\n"
                "}\n",

                SHADER_DEMO_HEADER
                "uniform float timeKeeper;\n"
                "uniform vec2 viewResolution;\n"
                "uniform float audioAmplitude;\n"
                "uniform float synthNoteColor;\n\n"
                "vec3 palette(float t)\n"
                "{\n"
                "   vec3 a = vec3(0.6, 0.4, 0.5);\n"
                "   vec3 b = vec3(0.7, 0.3, 0.5);\n"
                "   vec3 c = vec3(0.8, 1.0, 0.6);\n"
                "   vec3 d = vec3(0.76, 0.41, 0.96);\n\n"
                "   return a + b * cos(6.28318 * (c * t + d));\n"
                "}\n"
                "float field(in vec3 p,float s) {\n"
                "	float strength = 7. + .03 * log(1.e-6 + fract(sin(timeKeeper) * 4373.11));\n"
                "	float accum = s/4.;\n"
                "	float prev = 0.;\n"
                "	float tw = 0.;\n"
                "	for (int i = 0; i < 26; ++i) {\n"
                "		float mag = dot(p, p);\n"
                "		p = abs(p) / mag + vec3(-.5, -.4, -1.5);\n"
                "		float w = exp(-float(i) / 7.);\n"
                "		accum += w * exp(-strength * pow(abs(mag - prev), 2.2));\n"
                "		tw += w;\n"
                "		prev = mag;\n"
                "	}\n"
                "	return max(0., 5. * accum / tw - .7);\n"
                "}\n"
                "\n"
                "float field2(in vec3 p, float s) {\n"
                "	float strength = 7. + .03 * log(1.e-6 + fract(sin(timeKeeper) * 4373.11));\n"
                "	float accum = s/4.;\n"
                "	float prev = 0.;\n"
                "	float tw = 0.;\n"
                "	for (int i = 0; i < 18; ++i) {\n"
                "		float mag = dot(p, p);\n"
                "		p = abs(p) / mag + vec3(-.5, -.4, -1.5);\n"
                "		float w = exp(-float(i) / 7.);\n"
                "		accum += w * exp(-strength * pow(abs(mag - prev), 2.2));\n"
                "		tw += w;\n"
                "		prev = mag;\n"
                "	}\n"
                "	return max(0., 5. * accum / tw - .7);\n"
                "}\n"
                "\n"
                "vec3 nrand3( vec2 co )\n"
                "{\n"
                "	vec3 a = fract( cos( co.x*8.3e-3 + co.y )*vec3(1.3e5, 4.7e5, 2.9e5) );\n"
                "	vec3 b = fract( sin( co.x*0.3e-3 + co.y )*vec3(8.1e5, 1.0e5, 0.1e5) );\n"
                "	vec3 c = mix(a, b, 0.5);\n"
                "	return c;\n"
                "}\n"
                "\n"
                "void main() {\n"
                "    vec2 uv = 2. * gl_FragCoord.xy / viewResolution.xy - 1.;\n"
                "	vec2 uvs = uv * viewResolution.xy / max(viewResolution.x, viewResolution.y);\n"
                "	vec3 p = vec3(uvs / 4., 0) + vec3(1., -1.3, 0.);\n"
                "	p += .2 * vec3(sin(timeKeeper / 16.), sin(timeKeeper / 12.),  sin(timeKeeper / 128.));\n"
                "\n"
                "	float freqs[4];\n"
                "	freqs[0] = palette(synthNoteColor).x;\n"
                "	freqs[1] = palette(synthNoteColor + 1).x;\n"
                "	freqs[2] = palette(synthNoteColor + 2).x;\n"
                "	freqs[3] = palette(synthNoteColor + 3).x;\n"
                "\n"
                "	float t = field(p,freqs[2]);\n"
                "	float v = (1. - exp((abs(uv.x) - 1.) * 6.)) * (1. - exp((abs(uv.y) - 1.) * 6.));\n"
                "\n"
                "	vec3 p2 = vec3(uvs / (4.+sin(timeKeeper*0.11)*0.2+0.2+sin(timeKeeper*0.15)*0.3+0.4), 1.5) + vec3(2., -1.3, -1.);\n"
                "	p2 += 0.25 * vec3(sin(timeKeeper / 16.), sin(timeKeeper / 12.),  sin(timeKeeper / 128.));\n"
                "	float t2 = field2(p2,freqs[3]);\n"
                "	vec4 c2 = mix(.4, 1., v) * vec4(1.3 * t2 * t2 * t2 ,1.8  * t2 * t2 , t2* freqs[0], t2);\n"
                "\n"
                "	vec2 seed = p.xy * 2.0;\n"
                "	seed = floor(seed * viewResolution.x);\n"
                "	vec3 rnd = nrand3( seed );\n"
                "	vec4 starcolor = vec4(pow(rnd.y,40.0));\n"
                "\n"
                "	vec2 seed2 = p2.xy * 2.0;\n"
                "	seed2 = floor(seed2 * viewResolution.x);\n"
                "	vec3 rnd2 = nrand3( seed2 );\n"
                "	starcolor += vec4(pow(rnd2.y,40.0));\n"
                "\n"
                "	fragColor = mix(freqs[3]-.3, 1., v) * vec4(1.5*freqs[2] * t * t* t , 1.2*freqs[1] * t * t, freqs[3]*t, 1.0)+c2+starcolor;\n"
                "}\n"
            }
        };

        return juce::Array<ShaderPreset> (presets, juce::numElementsInArray (presets));
    }
};

class OpenGLComponent  : public juce::Component
                       , private juce::OpenGLRenderer
                       , private juce::AsyncUpdater
                       , private juce::Timer
{
public:
    OpenGLComponent()
    {
        if (auto* peer = getPeer())
            peer->setCurrentRenderingEngine (0);

        setOpaque (true);
        controlsOverlay.reset (new OpenGLComponentControlsOverlay (*this));
        addAndMakeVisible (controlsOverlay.get());

        openGLContext.setRenderer (this);
        openGLContext.attachTo (*this);
        openGLContext.setContinuousRepainting (true);

        controlsOverlay->initialise();

        startTimer(30);
        setSize (500, 500);
    }

    ~OpenGLComponent() override
    {
        openGLContext.detach();
    }

    void timerCallback() override
    {
        time += 0.030f;
        triggerAsyncUpdate();
    }

    void newOpenGLContextCreated() override
    {
        freeAllContextObjects();

        if (controlsOverlay.get() != nullptr)
            controlsOverlay->updateShader();
    }

    void openGLContextClosing() override
    {
        freeAllContextObjects();
    }

    void freeAllContextObjects()
    {
        shape     .reset();
        shader    .reset();
        attributes.reset();
        uniforms  .reset();
    }

    void renderOpenGL() override
    {
        using namespace ::juce::gl;

        const juce::ScopedLock lock (mutex);

        jassert (juce::OpenGLHelpers::isContextActive());

        auto desktopScale = (float) openGLContext.getRenderingScale();

        juce::OpenGLHelpers::clear (juce::Colours::black);

        updateShader();

        if (shader.get() == nullptr)
            return;

        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LESS);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glActiveTexture (GL_TEXTURE0);
        glEnable (GL_TEXTURE_2D);

        glViewport (0, 0,
                    juce::roundToInt (desktopScale * (float) bounds.getWidth()),
                    juce::roundToInt (desktopScale * (float) bounds.getHeight()));

        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        shader->use();

        if (uniforms->projectionMatrix.get() != nullptr)
            uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);

        if (uniforms->viewMatrix.get() != nullptr)
            uniforms->viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);

        if (uniforms->texture.get() != nullptr)
            uniforms->texture->set ((GLint) 0);

        if (uniforms->lightPosition.get() != nullptr)
            uniforms->lightPosition->set (-15.0f, 10.0f, 15.0f, 0.0f);

        if (uniforms->bouncingNumber.get() != nullptr)
            uniforms->bouncingNumber->set (bouncingNumber.getValue());

        if (uniforms->viewResolution.get() != nullptr)
            uniforms->viewResolution->set (getWidth(), getHeight());

        if (uniforms->timeKeeper.get() != nullptr)
            uniforms->timeKeeper->set(time);

        if (uniforms->audioAmplitude.get() != nullptr)
            uniforms->audioAmplitude->set(audioAmplitude);

        if (uniforms->synthNoteColor.get() != nullptr)
            uniforms->synthNoteColor->set(synthNoteColor);

        shape->draw (*attributes);

        // Reset the element buffers so child Components draw correctly
        glBindBuffer (GL_ARRAY_BUFFER, 0);
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);

        if (! controlsOverlay->isMouseButtonDownThreadsafe())
            rotation += (float) rotationSpeed;
    }

    juce::Matrix3D<float> getProjectionMatrix() const
    {
        const juce::ScopedLock lock (mutex);

        auto w = 1.0f / (scale + 0.1f);
        auto h = w * bounds.toFloat().getAspectRatio (false);

        return juce::Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);
    }

    juce::Matrix3D<float> getViewMatrix() const
    {
        const juce::ScopedLock lock (mutex);

        auto viewMatrix = juce::Matrix3D<float>::fromTranslation ({ 0.0f, 1.0f, -10.0f }) * draggableOrientation.getRotationMatrix();
        auto rotationMatrix = juce::Matrix3D<float>::rotation ({ rotation, rotation, -0.3f });

        return viewMatrix * rotationMatrix;
    }

    void setShaderProgram (const juce::String& vertexShader, const juce::String& fragmentShader)
    {
        const juce::ScopedLock lock (shaderMutex); // Prevent concurrent access to shader strings and status
        newVertexShader = vertexShader;
        newFragmentShader = fragmentShader;
    }

    void paint(juce::Graphics&) override {}

    void resized() override
    {
        const juce::ScopedLock lock (mutex);

        bounds = getLocalBounds();
        controlsOverlay->setBounds (bounds);
        //draggableOrientation.setViewport (bounds);
    }

    juce::Rectangle<int> bounds;
    juce::Draggable3DOrientation draggableOrientation;
    bool doBackgroundDrawing = false;
    float scale = 0.5f, rotationSpeed = 0.0f;
    BouncingNumber bouncingNumber;
    juce::CriticalSection mutex;
    float time = 0.0f, audioAmplitude = 1.0f, synthNoteColor = 0.0f;

private:
    void handleAsyncUpdate() override
    {
        const juce::ScopedLock lock (shaderMutex);
        controlsOverlay->statusLabel.setText (statusText, juce::dontSendNotification);
    }

    juce::OpenGLContext openGLContext;

    class OpenGLComponentControlsOverlay  : public Component
                                          , private juce::CodeDocument::Listener
                                          , private Timer
    {
    public:
        OpenGLComponentControlsOverlay (OpenGLComponent& d)
        :   demo (d)
        {
            addAndMakeVisible (statusLabel);
            statusLabel.setJustificationType (juce::Justification::topLeft);
            statusLabel.setFont (juce::Font (14.0f));

            addAndMakeVisible (tabbedComp);
            tabbedComp.setTabBarDepth (25);
            tabbedComp.setColour (juce::TabbedButtonBar::tabTextColourId, juce::Colours::grey);
            tabbedComp.addTab ("Vertex", juce::Colours::transparentBlack, &vertexEditorComp, false);
            tabbedComp.addTab ("Fragment", juce::Colours::transparentBlack, &fragmentEditorComp, false);

            vertexDocument.addListener (this);
            fragmentDocument.addListener (this);

            addAndMakeVisible (presetBox);
            presetBox.onChange = [this] { selectPreset (presetBox.getSelectedItemIndex()); };

            auto presets = OpenGLUtils::getPresets();

            for (int i = 0; i < presets.size(); ++i)
                presetBox.addItem (presets[i].name, i + 1);

            addAndMakeVisible (presetLabel);
            presetLabel.attachToComponent (&presetBox, false);

            lookAndFeelChanged();
        }

        void initialise()
        {
            presetBox .setSelectedItemIndex (0);
        }

        void resized() override
        {
            presetBox.setBounds (getWidth() - 120, 0, 120, 30);
        }

        bool isMouseButtonDownThreadsafe() const { return buttonDown; }

        void mouseDown (const juce::MouseEvent& e) override
        {
            const juce::ScopedLock lock (demo.mutex);
            demo.draggableOrientation.mouseDown (e.getPosition());

            buttonDown = true;
        }

        void mouseDrag (const juce::MouseEvent& e) override
        {
            const juce::ScopedLock lock (demo.mutex);
            demo.draggableOrientation.mouseDrag (e.getPosition());
        }

        void mouseUp (const juce::MouseEvent&) override
        {
            buttonDown = false;
        }

        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& d) override
        {
            sizeSlider.setValue (sizeSlider.getValue() + d.deltaY);
        }

        void mouseMagnify (const juce::MouseEvent&, float magnifyAmmount) override
        {
            sizeSlider.setValue (sizeSlider.getValue() + magnifyAmmount - 1.0f);
        }

        void selectPreset (int preset)
        {
            const auto& p = OpenGLUtils::getPresets()[preset];

            vertexDocument  .replaceAllContent (p.vertexShader);
            fragmentDocument.replaceAllContent (p.fragmentShader);

            startTimer (1);
        }

        void updateShader()
        {
            startTimer (10);
        }

        juce::Label statusLabel;

    private:
        enum { shaderLinkDelay = 500 };

        void codeDocumentTextInserted (const juce::String& /*newText*/, int /*insertIndex*/) override
        {
            startTimer (shaderLinkDelay);
        }

        void codeDocumentTextDeleted (int /*startIndex*/, int /*endIndex*/) override
        {
            startTimer (shaderLinkDelay);
        }

        void timerCallback() override
        {
            stopTimer();
            demo.setShaderProgram (vertexDocument  .getAllContent(),
                                   fragmentDocument.getAllContent());
        }

        void lookAndFeelChanged() override
        {
            auto editorBackground = getUIColourIfAvailable (juce::LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                                            juce::Colours::white);

            for (int i = tabbedComp.getNumTabs(); i >= 0; --i)
                tabbedComp.setTabBackgroundColour (i, editorBackground);

            vertexEditorComp  .setColour (juce::CodeEditorComponent::backgroundColourId, editorBackground);
            fragmentEditorComp.setColour (juce::CodeEditorComponent::backgroundColourId, editorBackground);
        }

        OpenGLComponent& demo;

        juce::Label speedLabel  { {}, "Speed:" },
              zoomLabel   { {}, "Zoom:" };

        juce::CodeDocument vertexDocument, fragmentDocument;
        juce::CodeEditorComponent vertexEditorComp    { vertexDocument,   nullptr },
                            fragmentEditorComp  { fragmentDocument, nullptr };

        juce::TabbedComponent tabbedComp              { juce::TabbedButtonBar::TabsAtLeft };

        juce::ComboBox presetBox, textureBox;

        juce::Label presetLabel   { {}, "Shader Preset:" };

        juce::Slider speedSlider, sizeSlider;

        juce::ToggleButton showBackgroundToggle  { "Draw 2D graphics in background" };

        std::unique_ptr<juce::FileChooser> textureFileChooser;

        std::atomic<bool> buttonDown { false };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponentControlsOverlay)
    };

    std::unique_ptr<OpenGLComponentControlsOverlay> controlsOverlay;

    float rotation = 0.0f;

    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    std::unique_ptr<OpenGLUtils::Shape> shape;
    std::unique_ptr<OpenGLUtils::Attributes> attributes;
    std::unique_ptr<OpenGLUtils::Uniforms> uniforms;

    juce::CriticalSection shaderMutex;
    juce::String newVertexShader, newFragmentShader, statusText;

    void updateShader()
    {
        const juce::ScopedLock lock (shaderMutex); // Prevent concurrent access to shader strings and status

        if (newVertexShader.isNotEmpty() || newFragmentShader.isNotEmpty())
        {
            std::unique_ptr<juce::OpenGLShaderProgram> newShader (new juce::OpenGLShaderProgram (openGLContext));

            if (newShader->addVertexShader (juce::OpenGLHelpers::translateVertexShaderToV3 (newVertexShader))
                  && newShader->addFragmentShader (juce::OpenGLHelpers::translateFragmentShaderToV3 (newFragmentShader))
                  && newShader->link())
            {
                shape     .reset();
                attributes.reset();
                uniforms  .reset();

                shader.reset (newShader.release());
                shader->use();

                shape     .reset (new OpenGLUtils::Shape      ());
                attributes.reset (new OpenGLUtils::Attributes (*shader));
                uniforms  .reset (new OpenGLUtils::Uniforms   (*shader));

                statusText = "GLSL: v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2);
            }
            else
            {
                statusText = newShader->getLastError();
            }

            triggerAsyncUpdate();

            newVertexShader   = {};
            newFragmentShader = {};
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponent)
};
