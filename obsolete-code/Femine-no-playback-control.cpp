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

/** shorthand macro for simplifying the reading of the left
 *  channel of a non-interleaved output buffer named out */
#define OUT_L out[0]

/** shorthand macro for simplifying the reading of the right
 *  channel of a non-interleaved output buffer named out */
#define OUT_R out[1]

/** shorthand macro for simplifying the reading of the left
 *  channel of a non-interleaved input buffer named in */
#define IN_L in[0]

/** shorthand macro for simplifying the reading of the right
 *  channel of a non-interleaved input buffer named in */
#define IN_R in[1]

// This runs at a fixed rate, to prepare audio samples
// good interleaving vs non-interleaving explanation:
// https://forum.electro-smith.com/t/what-is-the-difference-between-the-two-types-of-audio-callback/454/3
// libfmsynth renders in non-interleaving way, so using that type of callback.
// libdaisy_reference.pdf describes Non-Interleaving AudioCallback format as "Both arrays arranged by float[chn][sample]"
// https://electro-smith.github.io/libDaisy/classdaisy_1_1_audio_handle.html#a10a5b51605fba6bb19ab3f05394da38f
void callback( // AudioHandle::InterleavingInputBuffer in,
               // AudioHandle::InterleavingOutputBuffer out,
    AudioHandle::InputBuffer in,
    AudioHandle::OutputBuffer out,
    size_t size)
{
    //
    // code from libfmsynth/lv2/fmsynth_lv2.cpp
    //
    // fmsynth_render(fm, left, right, to_render);
    // fmsynth_parse_midi(fm, body);
    // fmsynth_set_global_parameter(fm, FMSYNTH_GLOBAL_PARAM_VOLUME, get_param(peg_volume));
    // fmsynth_set_global_parameter(fm, FMSYNTH_GLOBAL_PARAM_LFO_FREQ, get_param(peg_lfofreq));
    // fmsynth_set_parameter(fm, i, o, get_param(i + base_port));
    /**/

    // https://en.cppreference.com/w/cpp/string/byte/memset
    std::memset(OUT_L, 0, FM_SAMPLE_RATE * sizeof(float));
    std::memset(OUT_R, 0, FM_SAMPLE_RATE * sizeof(float));

    // TODO detect gate on/off and send note on/off
    //
    // Parameters
    // fm	Handle to an FM synth instance.
    // note	Which note to press. Note is parsed using MIDI rules, i.e. note = 69 is A4. Valid range is [0, 127].
    // velocity	Note velocity. Velocity is parsed using MIDI rules. valie range is [0, 127].
    // ./libfmsynth/docs/html/group__libfmsynthControl.html#ga8186cfec9cf2266289358a14ddb24b31
    fmsynth_note_on(fm, 69, 127);

    // one-liner
    // Parameters
    // fm	Handle to an FM synth instance.
    // left	A pointer to buffer representing the left channel.
    // right	A pointer to buffer representing the right channel.
    // frames	The number of frames (left and right samples) to render.
    // ./libfmsynth/docs/html/group__libfmsynthRender.html
    fmsynth_render(fm, OUT_L, OUT_R, size);

    // for ( // using iterator
    //       // get first item
    //     LV2_Atom_Event *ev = lv2_atom_sequence_begin(&seq->body);

    //     // check if end of array
    //     !lv2_atom_sequence_is_end(&seq->body, seq->atom.size, ev);

    //     // get next
    //     ev = lv2_atom_sequence_next(ev)

    // )
    // {

    // From https://lv2plug.in/c/html/group__atom.html#unionLV2__Atom__Event_8time
    // int64_t 	frames 	Time in audio frames.
    // uint32_t to = size; // ev->time.frames;
    // uint32_t samples_done = 0;
    // int sample_count = FM_SAMPLE_RATE;

    // if (to > samples_done)
    // {
    //     uint32_t to_render = to - samples_done;
    //     fmsynth_render(fm, OUT_L, OUT_R, to_render);

    //     samples_done += to_render;
    //     OUT_L += to_render;
    //     OUT_R += to_render;
    // }
    // // }

    // if (sample_count > samples_done)
    // {
    //     uint32_t to_render = sample_count - samples_done;
    //     fmsynth_render(fm, OUT_L, OUT_R, to_render);
    // }

    //
    //
    //
    /**/
    // Audio is (WAS; NOW NOT) interleaved stereo by default, so not using `i + 1`.
    // for (size_t i = 0; i < size; i += 2)
    // {
    //     // left out
    //     OUT_L[i] = decimator_l.Process(IN_L[i]);

    //     // right out
    //     OUT_R[i] = decimator_r.Process(IN_R[i]);
    // }
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

        // decimator_l.SetBitcrushFactor(hw.GetKnobValue(DaisyVersio::KNOB_0));
        // decimator_l.SetDownsampleFactor(hw.GetKnobValue(DaisyVersio::KNOB_1));

        // decimator_r.SetBitcrushFactor(hw.GetKnobValue(DaisyVersio::KNOB_2));
        // decimator_r.SetDownsampleFactor(hw.GetKnobValue(DaisyVersio::KNOB_3));

        fmsynth_set_global_parameter(fm, FMSYNTH_GLOBAL_PARAM_VOLUME, hw.GetKnobValue(DaisyVersio::KNOB_0));
        fmsynth_set_global_parameter(fm, FMSYNTH_GLOBAL_PARAM_LFO_FREQ, hw.GetKnobValue(DaisyVersio::KNOB_1));

        kval = hw.GetKnobValue(DaisyVersio::KNOB_6);
    }
}
