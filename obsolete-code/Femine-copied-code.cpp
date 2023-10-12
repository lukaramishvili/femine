#include "./libfmsynth/include/fmsynth.h"
#include "./libfmsynth/include/fmsynth_private.h"

#include "daisy_versio.h"
#include "daisysp.h"
#include <string>

using namespace daisy;
using namespace daisysp;

DaisyVersio hw;

// bitcrush effect
Decimator decimator_l;
Decimator decimator_r;

static float kval;

fmsynth_t *fm;
int FM_SAMPLE_RATE = 44100;
int FM_MAX_VOICES = 128;

// This runs at a fixed rate, to prepare audio samples
void callback(AudioHandle::InterleavingInputBuffer in,
              AudioHandle::InterleavingOutputBuffer out,
              size_t size)
{
    // code from libfmsynth/lv2/fmsynth_lv2.cpp
    //
    // fmsynth_render(fm, left, right, to_render);
    // fmsynth_parse_midi(fm, body);
    // fmsynth_set_global_parameter(fm, FMSYNTH_GLOBAL_PARAM_VOLUME, get_param(peg_volume));
    // fmsynth_set_global_parameter(fm, FMSYNTH_GLOBAL_PARAM_LFO_FREQ, get_param(peg_lfofreq));
    // fmsynth_set_parameter(fm, i, o, get_param(i + base_port));
    /**/
    const LV2_Atom_Sequence *seq = p<LV2_Atom_Sequence>(peg_midi);
    uint32_t samples_done = 0;

    float *left = p(peg_output_left);
    float *right = p(peg_output_right);

    std::memset(left, 0, sample_count * sizeof(float));
    std::memset(right, 0, sample_count * sizeof(float));

    for (LV2_Atom_Event *ev = lv2_atom_sequence_begin(&seq->body);
         !lv2_atom_sequence_is_end(&seq->body, seq->atom.size, ev);
         ev = lv2_atom_sequence_next(ev))
    {
        update_parameters();
        uint32_t to = ev->time.frames;

        if (to > samples_done)
        {
            uint32_t to_render = to - samples_done;
            fmsynth_render(fm, left, right, to_render);

            samples_done += to_render;
            left += to_render;
            right += to_render;
        }
    }

    if (sample_count > samples_done)
    {
        uint32_t to_render = sample_count - samples_done;
        fmsynth_render(fm, left, right, to_render);
    }
    //
    //
    //
    /**/
    // Audio is interleaved stereo by default
    for (size_t i = 0; i < size; i += 2)
    {
        // left out
        out[i] = decimator_l.Process(in[i]);

        // right out
        out[i + 1] = decimator_r.Process(in[i + 1]);
    }
}

int main(void)
{
    // Initialize Versio hardware and start audio, ADC
    hw.Init();

    decimator_l.Init();
    decimator_r.Init();

    kval = 0.f;

    hw.StartAudio(callback);
    hw.StartAdc();

    // Initialize FM synth engine
    fm = fmsynth_new(FM_SAMPLE_RATE, FM_MAX_VOICES);

    if (fm == nullptr)
    {
        // signal error with LED signals: OFF RED OFF RED
        hw.SetLed(DaisyVersio::LED_0, 0, 0, 0);
        hw.SetLed(DaisyVersio::LED_1, 255, 0, 0);
        hw.SetLed(DaisyVersio::LED_2, 0, 0, 0);
        hw.SetLed(DaisyVersio::LED_3, 255, 0, 0);
    }

    while (1)
    {
        hw.ProcessAnalogControls(); // Normalize CV inputs
        hw.UpdateExample();         // Control the LED colors using the knobs and gate inputs
        hw.UpdateLeds();

        decimator_l.SetBitcrushFactor(hw.GetKnobValue(DaisyVersio::KNOB_0));
        decimator_l.SetDownsampleFactor(hw.GetKnobValue(DaisyVersio::KNOB_1));

        decimator_r.SetBitcrushFactor(hw.GetKnobValue(DaisyVersio::KNOB_2));
        decimator_r.SetDownsampleFactor(hw.GetKnobValue(DaisyVersio::KNOB_3));

        kval = hw.GetKnobValue(DaisyVersio::KNOB_6);
    }
}
