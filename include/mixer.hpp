#pragma once

#include <stdio.h>
#include <stdint.h>
#include <alsa/asoundlib.h>
#include <thread>
#include <mutex>

namespace mixer {
 
#define PCM_DEVICE "default"
#define SAMPLE_RATE 11025
#define FRAMES 4096
#define PERIOD 128
  //4096 is 4096/11025 = 371.52ms @ 11 KHz
#define FLOAT 32767.f
  //(1 << (sizeof(short)-1))-1
#define MAX_NUM_MIXER_TRACKS 16
#define NUM_CHANNELS 1
#define BITS_PER_SAMPLE 16

#pragma pack(1)
  struct WAV_HEADER
  {
    char ChunkID[4];
    uint32_t ChunkSize;
    char Format[4];
    char Subchunk1ID[4];
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    char Subchunk2ID[4];
    uint32_t Subchunk2Size;
  };
#pragma pack(8)

  class Mixer
  {
  public:
    bool create(int num_mixer_tracks = 4, float master_volume = 1.0f);
    bool load(const char * filename, int track, float volume = 1.0f);
    void volume(int track, float volume);
    void master_volume(float volume = 1.0f);
    void master_volume_div(float divisor = 1.0f);
    void master_volume_per(float multiplier = 1.0f);
    void pause(void);
    void resume(void);
    bool is_running(void);
    void stop(void);
  
  private:
    void loop(void);

    snd_pcm_t* m_pcm_handle = NULL;
    std::thread  m_main_loop;
    std::mutex m_play_lock;
    int m_mixer_tracks = 0;
    float m_master_volume = 0.0f;
    short m_master_buffer[FRAMES];
    float *m_volume = NULL;
    int   *m_index  = NULL;
    int   *m_max_index = NULL;
    float **m_buffer   = NULL;
  };

  void Mixer::loop(void)
  {
    int err;
    float mix;
  
    while (true) {

      //Polling
      if ((err = snd_pcm_wait(m_pcm_handle, -1)) < 0)
	{
	  fprintf (stderr, "Poll failed (%s)\n", snd_strerror (err));
	  break;
	}      
	
      //Deliver the data
      std::lock_guard<std::mutex> lock(m_play_lock);

      //TODO: modify to add handling of xrun
      while (((snd_pcm_avail_update(m_pcm_handle)) >= PERIOD) && (snd_pcm_state(m_pcm_handle) !=  SND_PCM_STATE_SETUP))
	{
	  for (int i = 0; i < PERIOD; i++)
	    {
	      //Mix the tracks
	      mix = 0.0f;
	      for (int j = 0; j < m_mixer_tracks; j++)
		{
		  mix += m_volume[j] * m_buffer[j][m_index[j]];
		  m_index[j] = (m_index[j] + 1)%m_max_index[j];
		}
	
	      //Hard clip
	      mix *= m_master_volume;
	      if (mix > 1.0f) mix = 1.0f;
	      if (mix < -1.0f) mix = -1.0f;

	      m_master_buffer[i] = (short)(mix * FLOAT);
	    }

	  if ((err = snd_pcm_writei(m_pcm_handle, m_master_buffer, PERIOD)) < 0)
	    {
	      fprintf (stderr, "pcm write failed (%s)\n", snd_strerror (err));
	    }
	}
      if (snd_pcm_state(m_pcm_handle) ==  SND_PCM_STATE_SETUP)
	break;
    }
  }

  bool Mixer::create(int num_mixer_tracks, float master_volume)
  {
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;

    //Check if we created the mixer already
    if (m_pcm_handle != NULL)
      {
	fprintf(stderr, "Mixer already created!\n");
	return false;
      }

    if ((num_mixer_tracks <= 0) || (num_mixer_tracks > MAX_NUM_MIXER_TRACKS))
      {
	fprintf (stderr, "Failed to open pcm device (Number of requested tracks not acceptable)\n");
	return false;
      }
    m_mixer_tracks = num_mixer_tracks;

    if ((master_volume < 0.0f) || (master_volume > 1.0f))
      {
	fprintf (stderr, "Failed to open pcm device (Master volume not acceptable)\n");
	return false;
      }
    m_master_volume = master_volume;

    m_volume    = new float [num_mixer_tracks];
    m_index     = new   int [num_mixer_tracks];
    m_max_index = new   int [num_mixer_tracks];
    m_buffer    = new float*[num_mixer_tracks];
    for (int i = 0; i < num_mixer_tracks; i++)
      {
	m_volume[i]    = 0.0f;
	m_index[i]     = 0;
	m_max_index[i] = 1;
	m_buffer[i] = new float[1];
	m_buffer[i][0] = 0.0f;
      }
  
    snd_pcm_open(&m_pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_hw_params_malloc(&hw_params);
    snd_pcm_hw_params_any(m_pcm_handle, hw_params);
    snd_pcm_hw_params_set_access(m_pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(m_pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(m_pcm_handle, hw_params,NUM_CHANNELS);
    snd_pcm_hw_params_set_rate(m_pcm_handle, hw_params, SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_buffer_size(m_pcm_handle, hw_params, FRAMES);
    snd_pcm_hw_params_set_period_size(m_pcm_handle, hw_params, PERIOD, 0);
    snd_pcm_hw_params(m_pcm_handle, hw_params);
    snd_pcm_hw_params_free(hw_params);

    snd_pcm_sw_params_malloc(&sw_params);
    snd_pcm_sw_params_current(m_pcm_handle, sw_params);
    snd_pcm_sw_params_set_avail_min(m_pcm_handle, sw_params, PERIOD);
    snd_pcm_sw_params_set_start_threshold(m_pcm_handle, sw_params, FRAMES - PERIOD);
    snd_pcm_sw_params(m_pcm_handle, sw_params);
    snd_pcm_sw_params_free(sw_params);
  
    snd_pcm_prepare(m_pcm_handle);
  
    //Endless loop
    if (snd_pcm_state(m_pcm_handle) == SND_PCM_STATE_PREPARED)
      m_main_loop = std::thread(&Mixer::loop, this);

    return (m_main_loop.joinable());
  }

  bool Mixer::load(const char * filename, int track, float volume)
  {
    FILE *wavfile;
    WAV_HEADER w;
    unsigned char *wave_data;
    
    //Check if we created the mixer already
    if (m_pcm_handle == NULL)
      {
	fprintf(stderr, "Must create a mixer first!\n");
	return false;
      }
      
    //Check input data
    if ((track < 1) || (track > m_mixer_tracks))
      {
	fprintf(stderr, "Requested track number %d not correct.\n", track);
	return false;
      }
    track--;

    if ((volume < 0.0f) || (volume > 1.0f))
      {
	fprintf(stderr, "Requested volume %f not correct.\n", volume);
	return false;
      }
    
    //Open given WAV file 
    wavfile = fopen(filename, "r");
    if (!wavfile)
      {
#ifdef GL_DEBUG
	fprintf(stderr, "<!> Could not open file \"%s\"\n", filename);
#endif
	return false;
      }

    //Read and check header
    fread(&w, sizeof(WAV_HEADER), 1, wavfile);
#ifdef GL_DEBUG
    if ((w.ChunkID[0] != 'R') || (w.ChunkID[1] != 'I') || (w.ChunkID[2] != 'F') || (w.ChunkID[3] != 'F'))
      {
	fprintf(stderr, "<!> %s is not a valid RIFF file.\n", filename);
	return false;
      }
    if ((w.Format[0] != 'W') || (w.Format[1] != 'A') || (w.Format[2] != 'V') || (w.Format[3] != 'E'))
      {
	fprintf(stderr, "<!> %s does not contain a format sub-chunck.\n", filename);
	return false;
      }
    if ((w.Subchunk1ID[0] != 'f') || (w.Subchunk1ID[1] != 'm') || (w.Subchunk1ID[2] != 't') || (w.Subchunk1ID[3] != ' '))
      {
	fprintf(stderr, "<!> %s does not contain a format sub-chunck.\n", filename);
	return false;
      }
#endif
    if ((w.Subchunk2ID[0] == 'L') || (w.Subchunk2ID[1] == 'I') || (w.Subchunk2ID[2] == 'S') || (w.Subchunk2ID[3] == 'T'))
      {
	//We don't care about the LIST section content
	fprintf(stderr, "<W> Skipping LIST sub-chunck in %s.\n", filename);
	fseek(wavfile, w.Subchunk2Size, SEEK_CUR);
	fread(&w.Subchunk2ID, sizeof(w.Subchunk2ID), 1, wavfile);
	fread(&w.Subchunk2Size, sizeof(w.Subchunk2Size), 1, wavfile);
      }
#ifdef GL_DEBUG
    if ((w.Subchunk2ID[0] != 'd') || (w.Subchunk2ID[1] != 'a') || (w.Subchunk2ID[2] != 't') || (w.Subchunk2ID[3] != 'a'))
      {
	fprintf(stderr, "<!> %s does not contain a data sub-chunck.", filename);
	return false;
      }
      
    //We only process a single format
    if (w.BitsPerSample != BITS_PER_SAMPLE)
      {
	fprintf(stderr, "<!> %s: Cannot play this WAV sample (Bits per sample is %d, must be %d).\n",
		filename, w.BitsPerSample, BITS_PER_SAMPLE);
	return false;
      }

    if (w.NumChannels != NUM_CHANNELS)
      {
	fprintf(stderr, "<!> %s: Cannot play this WAV sample (Number of channels is %d, must be %d).\n",
		filename, w.NumChannels, NUM_CHANNELS);
	return false;
      }
    if (w.AudioFormat != 1)
      {
	fprintf(stderr, "<!> %s: Cannot play this WAV sample (Not PCM).\n", filename);
	return false;
      }
 
    if (w.SampleRate != SAMPLE_RATE)
      {
	fprintf(stderr, "<!> %s: Cannot play this WAV sample (Sample rate is %d (must be %d)).\n",
		filename, w.SampleRate, SAMPLE_RATE);
	return false;
      }
#endif
    
    //Read the raw data
    wave_data = new unsigned char [w.Subchunk2Size];
#ifdef GL_DEBUG    
    if (!wave_data)
      {
	fprintf(stderr, "<!> Could not allocate wave data.\n");
	return false;
      }
#endif
    
    if(fread(wave_data, w.Subchunk2Size, 1, wavfile) != 1)
#ifdef GL_DEBUG
      {
	fprintf(stderr, "<!> Error reading wave data.\n");
	return false;
      }
#endif
    fclose(wavfile);

    std::lock_guard<std::mutex> lock(m_play_lock);

    m_index[track] = 0;
    m_volume[track] = volume;
    m_max_index[track] = w.Subchunk2Size * 8 / (w.NumChannels * w.BitsPerSample);
    if (m_buffer[track])
      delete [] m_buffer[track];
    m_buffer[track] = new float [m_max_index[track]];

    for (int i = 0, k = 0; i < m_max_index[track]; i++, k++)
      {
	if (w.BitsPerSample == 8)
	  {
	    m_buffer[track][i] = (float)(wave_data[k]/0xFF);
	  }
	else
	  {
	    m_buffer[track][i] = (short)(wave_data[k] | wave_data[k+1] << 8) / FLOAT;
	    k++;
	  }
      }

    delete [] wave_data;

    return true;
  }

  void Mixer::volume(int track, float volume)
  {
    //Check input data
    if ((track < 1) || (track > m_mixer_tracks))
      {
	fprintf(stderr, "Requested track number %d not correct.\n", track);
	return;
      }
    track--;
    
    if ((volume < 0.0f) || (volume > 1.0f))
      {
	fprintf(stderr, "Requested volume %f not correct.\n", volume);
	return;
      }

    std::lock_guard<std::mutex> lock(m_play_lock);
    if (m_volume != NULL)
      m_volume[track] = volume;
  }

  void Mixer::master_volume(float volume)
  {
    //Check input data
    if ((volume < 0.0f) || (volume > 1.0f))
      {
	fprintf(stderr, "Requested volume %f not correct.\n", volume);
	return;
      }

    std::lock_guard<std::mutex> lock(m_play_lock);
    m_master_volume = volume;
  }

  void Mixer::master_volume_div(float divisor)
  {
    //Check input data
    if (divisor <= 0.0f)
      {
	fprintf(stderr, "Requested divisor %f not correct.\n", divisor);
	return;
      }

    std::lock_guard<std::mutex> lock(m_play_lock);
    m_master_volume /= divisor;
  }

  void Mixer::master_volume_per(float multiplier)
  {
    //Check input data
    if (multiplier <= 0.0f)
      {
	fprintf(stderr, "Requested multiplier %f not correct.\n", multiplier);
	return;
      }

    std::lock_guard<std::mutex> lock(m_play_lock);
    m_master_volume *= multiplier;
    if (m_master_volume > 1.0)
      m_master_volume = 1.0f;
  }

  /*
   * This only works if the hardware supports the pause feature.
   * Should we check this via the snd_pcm_hw_params_can_pause() function?
   *
   */
  void Mixer::pause(void)
  {
    if (snd_pcm_state(m_pcm_handle) == SND_PCM_STATE_RUNNING)
      snd_pcm_pause(m_pcm_handle, 1);
  }

  void Mixer::resume(void)
  {
    if (snd_pcm_state(m_pcm_handle) == SND_PCM_STATE_PAUSED)
      snd_pcm_pause(m_pcm_handle, 0);
  }

  bool Mixer::is_running(void)
  {
    return (snd_pcm_state(m_pcm_handle) == SND_PCM_STATE_RUNNING);
  }

  void Mixer::stop(void)
  {
    if (m_pcm_handle)
      {
	snd_pcm_drop(m_pcm_handle);
	if (m_main_loop.joinable())
	  m_main_loop.join();
	snd_pcm_close(m_pcm_handle);
	m_pcm_handle = NULL;
	m_master_volume = 0.0f;

	if (m_volume)
	  delete [] m_volume;
	if (m_index)
	  delete [] m_index;
	if (m_max_index)
	  delete [] m_max_index;
	for (int i = 0; i < m_mixer_tracks; i++)
	  {
	    if (m_buffer[i])
	      delete [] m_buffer[i];
	  }
	m_mixer_tracks = 0;
	if (m_buffer)
	  delete [] m_buffer;
      }
  }
  
}
