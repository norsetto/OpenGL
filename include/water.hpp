#include <cstdlib>

#include "glad.h"

#include <cmath>
#include <ctime>

#include <glm/glm.hpp>

namespace water {
#define NUM_WAVES 4

  struct WAVE_DATA
  {
    float amplitude[NUM_WAVES];
    float frequency[NUM_WAVES];
    float phase[NUM_WAVES];
    float Q[NUM_WAVES];
    glm::vec2 D[NUM_WAVES];
  };

  class Water
  {
  private:

    inline float random(float min, float delta)
    {
      return min + delta * static_cast<float>(rand()) / RAND_MAX;
    }

  public:

    Water();
    ~Water();
    void generate(int index);
    void generate(void);
    void update(float deltaTime);
    void render(GLsizei num_patches);
    WAVE_DATA *data(void);
    void roughness(float sea_roughness);
    void wavelength(float sea_wavelength);
    void amplitude(float min_amplitude, float max_amplitude);
    void lifetime(float min_lifetime, float max_lifetime);
    void wind(glm::vec2& direction, float variation);
  
  private:
  
    GLuint m_quad_vao = 0;
    WAVE_DATA m_wave_data;
    float m_sea_roughness = 0.15f, m_sea_wavelength = 40.0f;
    float m_min_amplitude = 0.05f, m_max_amplitude = 0.15f;
    float m_min_lifetime = 30.0f, m_max_lifetime = 60.0f;
    float m_lifetime[NUM_WAVES];
    glm::vec2 m_wind_direction = { 1.0f, 0.0f };
    float m_direction_variation = 0.3f;
  };

Water::Water()
{
  srand (static_cast <unsigned> (time(0)));
}

Water::~Water()
{
  if (m_quad_vao != 0)
    glDeleteVertexArrays(1, &m_quad_vao);
}

void Water::generate(int index)
{
  m_wave_data.amplitude[index] =  Water::random(m_min_amplitude, m_max_amplitude - m_min_amplitude);
  m_wave_data.frequency[index] = 6.28318530718f * Water::random(0.5f, 1.5f) / m_sea_wavelength;
  m_wave_data.phase[index]     = std::sqrt(10.0f * m_wave_data.frequency[index]);
  m_wave_data.Q[index]         = m_sea_roughness *
    Water::random( 0.0f, 1.0f / (m_wave_data.frequency[index] * m_wave_data.amplitude[index]));

  float wind_angle = std::atan2(m_wind_direction.y, m_wind_direction.x);
  float angle = Water::random(wind_angle - m_direction_variation, 2.0f * m_direction_variation);
  m_wave_data.D[index] = glm::vec2(std::cos(angle), std::sin(angle));

  //  printf("Generated new wave #%d\n", index);
}
  
void Water::generate(void)
{

  if (m_quad_vao == 0)
    glGenVertexArrays(1, &m_quad_vao);

  for (int i = 0; i < NUM_WAVES; i++)
    {
      Water::generate(i);
      m_lifetime[i] = Water::random(m_min_lifetime, m_max_lifetime - m_min_lifetime);
    }
}

void Water::update(float deltaTime)
{
  static float fade[NUM_WAVES], amp[NUM_WAVES];
  const float delta_fade = 0.005f; // 200 frames

  for (int i = 0; i < NUM_WAVES; i++)
    {
      if (m_lifetime[i] > 0.0f)
	{
	  m_lifetime[i] -= deltaTime;
	  if (m_lifetime[i] <= 0.0f)
	    {
	      m_lifetime[i] = 0.0f;
	      fade[i] = 1.0f;
	      amp[i] = m_wave_data.amplitude[i];
	    }
	}
      else
	{
	  if (fade[i] >= 0.0f)
	    {
	      fade[i] -= delta_fade;
	      if (fade[i] >= 0.0f)
		{
		  m_wave_data.amplitude[i] = fade[i] * amp[i];
		}
	      else
		{
		  fade[i] = -1.0f;
		  Water::generate(i);
		  amp[i] = m_wave_data.amplitude[i];
		}
	    }
	  else
	    {
	      fade[i] += delta_fade;
	      if (fade[i] < 0.0f)
		{
		  m_wave_data.amplitude[i] = (1.0f + fade[i]) * amp[i];
		}
	      else
		{
		  m_wave_data.amplitude[i] = amp[i];
		  m_lifetime[i] = Water::random(m_min_lifetime, m_max_lifetime - m_min_lifetime);
		}
	    }
	}
    }
}

void Water::render(GLsizei num_patches)
{
  glBindVertexArray(m_quad_vao);
  glPatchParameteri(GL_PATCH_VERTICES, 4);
  glDrawArraysInstanced(GL_PATCHES, 0, 4, num_patches);
}

WAVE_DATA *Water::data(void)
{
  return &m_wave_data;
}

void Water::roughness(float sea_roughness)
{
  if ((sea_roughness >= 0.0f) && (sea_roughness <= 1.0f))
    m_sea_roughness = sea_roughness;
}

void Water::wavelength(float sea_wavelength)
{
  if (sea_wavelength > 0.0f)
    m_sea_wavelength = sea_wavelength;
}

void Water::amplitude(float min_amplitude, float max_amplitude)
{
  if ((max_amplitude > min_amplitude) && (max_amplitude > 0.0f) && (min_amplitude >= 0.0f))
    {
      m_min_amplitude = min_amplitude;
      m_max_amplitude = max_amplitude;
    }
}

void Water::lifetime(float min_lifetime, float max_lifetime)
{
  if ((max_lifetime > min_lifetime) && (max_lifetime > 0.0f) && (min_lifetime >= 0.0f))
    {
      m_min_lifetime = min_lifetime;
      m_max_lifetime = max_lifetime;
    }
}

void Water::wind(glm::vec2& direction, float variation)
{
  m_wind_direction = glm::normalize(direction);
  if (variation >= 0.0f)
    m_direction_variation = variation;
}

}
