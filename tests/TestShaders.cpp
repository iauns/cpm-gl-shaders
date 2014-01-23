/// \author James Hughes
/// \date   November 2013

#include <batch-testing/GlobalGTestEnv.hpp>
#include <batch-testing/ContextTestFixture.hpp>

#include <gl-shaders/GLShader.hpp>
#include <file-util/FileUtil.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>   // glm::translate, glm::rotate, glm::perspective
#include <glm/gtc/type_ptr.hpp>           // glm::value_ptr

using namespace CPM_BATCH_TESTING_NS;
namespace gls = CPM_GL_SHADERS_NS;

TEST_F(ContextTestFixture, TestBasicRendering)
{
  // Since we are using ContextTestFixture, a context has already been created
  // for us by the time that we get here.

  std::vector<float> vboData = 
  {
    // Color (aColorFloat)     position (aPos)
     0.0f, 1.0f, 0.0f, 1.0f,  -1.0f,  1.0f,  0.0f,
     0.0f, 1.0f, 0.0f, 1.0f,   1.0f,  1.0f,  0.0f,
     0.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,  0.0f,
     0.0f, 1.0f, 0.0f, 1.0f,   1.0f, -1.0f,  0.0f,  
  };

  // 16-bit iboData
  std::vector<uint16_t> iboData =
  {
    0, 1, 2, 3
  };

  std::string vertexShader   = CPM_FILE_UTIL_NS::readFile("shaders/Color.vsh");
  std::string fragmentShader = CPM_FILE_UTIL_NS::readFile("shaders/Color.fsh");

  // Compile and link vertex and fragment shader into a program.
  GLuint program = CPM_GL_SHADERS_NS::loadShaderProgram(
      {
        gls::ShaderSource({vertexShader.c_str()}, GL_VERTEX_SHADER),
        gls::ShaderSource({fragmentShader.c_str()}, GL_FRAGMENT_SHADER),
      });

  std::string nameInCode;

  // Retrieve information regarding attributes.
  std::vector<gls::ShaderAttribute> attribs = gls::getProgramAttributes(program);

  // Sort attributes alphebetically so we know what order they will come in.
  // Also useful when binding attributes programatically.
  gls::sortAttributesAlphabetically(attribs);

  ASSERT_EQ(2, attribs.size());

  nameInCode = attribs[0].nameInCode;
  EXPECT_EQ(GL_FLOAT_VEC4, attribs[0].type);
  EXPECT_EQ(1, attribs[0].size);
  EXPECT_EQ("aColorFloat", nameInCode);

  nameInCode = attribs[1].nameInCode;
  EXPECT_EQ(GL_FLOAT_VEC3, attribs[1].type);
  EXPECT_EQ(1, attribs[1].size);
  EXPECT_EQ("aPos", nameInCode);

  // Retrieve information regarding uniforms.
  std::vector<gls::ShaderUniform> uniforms = gls::getProgramUniforms(program);
  
  // Ensure uProjIVObject is the only uniform.
  ASSERT_EQ(1, uniforms.size());
  nameInCode = uniforms[0].nameInCode;
  EXPECT_EQ(GL_FLOAT_MAT4, uniforms[0].type);
  EXPECT_EQ(1, uniforms[0].size);
  EXPECT_EQ("uProjIVObject", nameInCode);

  // Construct VBO.
  GLuint vbo;
  GL(glGenBuffers(1, &vbo));
  GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  GL(glBufferData(GL_ARRAY_BUFFER, 
                  static_cast<GLsizeiptr>(vboData.size() * sizeof(float)),
                  reinterpret_cast<GLvoid*>(&vboData[0]), GL_STATIC_DRAW));

  // Construct IBO
  GLuint ibo;
  GL(glGenBuffers(1, &ibo));
  GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
  GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                  static_cast<GLsizeiptr>(iboData.size() * sizeof(uint16_t)),
                  reinterpret_cast<GLvoid*>(&iboData[0]), GL_STATIC_DRAW));

  //-----------------
  // Render the quad
  //-----------------

  // This function is from our test fixture ContextTestFixture from
  // cpm-gl-batch-testing.
  beginFrame();

  // Bind shader
  GL(glUseProgram(program));

  // Bind VBO and IBO
  GL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

  // Build projection matrix looking down negative Z.
  float aspect = static_cast<float>(640) / static_cast<float>(480);
  glm::mat4 projection = glm::perspective(0.59f, aspect, 1.0f, 2000.0f);

  // Assign projection to appropriate uniform (the only uniform currently).
  GL(glUniformMatrix4fv(static_cast<GLint>(uniforms[0].uniformLoc), 1, false,
                        static_cast<const GLfloat*>(glm::value_ptr(projection))));

  // Properly assign attributes. We have already sorted the uniforms in
  // alphabetical order, and we have ensured that the VBO was built in
  // alphabetical order. So all we have to do is bind the attributes according
  // to the sorted attribute vector.
  gls::bindAllAttributes(&attribs[0], attribs.size());

  GL(glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(iboData.size()),
                    GL_UNSIGNED_SHORT, 0));

  gls::unbindAllAttributes(&attribs[0], attribs.size());
}


