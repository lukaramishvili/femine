#include "./libfmsynth/include/fmsynth.h"
#include "./libfmsynth/include/fmsynth_private.h"

#include "daisy_versio.h"
#include "daisysp.h"
#include <string>

using namespace daisy;
using namespace daisysp;

DaisyVersio hw;

static float basePitch;
static float finalPitch;

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

constexpr Pin PIN_TRIG_IN = seed::D24;
constexpr Pin PIN_SW = seed::D30;
constexpr Pin PIN_TOGGLE3_0A = seed::D6;
constexpr Pin PIN_TOGGLE3_0B = seed::D5;
constexpr Pin PIN_TOGGLE3_1A = seed::D1;
constexpr Pin PIN_TOGGLE3_1B = seed::D0;

constexpr Pin PIN_LED0_R = seed::D10;
constexpr Pin PIN_LED0_G = seed::D3;
constexpr Pin PIN_LED0_B = seed::D4;
constexpr Pin PIN_LED1_R = seed::D12;
constexpr Pin PIN_LED1_G = seed::D13;
constexpr Pin PIN_LED1_B = seed::D11;
constexpr Pin PIN_LED2_R = seed::D25;
constexpr Pin PIN_LED2_G = seed::D26;
constexpr Pin PIN_LED2_B = seed::D14;
constexpr Pin PIN_LED3_R = seed::D29;
constexpr Pin PIN_LED3_G = seed::D27;
constexpr Pin PIN_LED3_B = seed::D15;

constexpr Pin PIN_ADC_CV0 = seed::D21;
constexpr Pin PIN_ADC_CV1 = seed::D22;
constexpr Pin PIN_ADC_CV2 = seed::D28;
constexpr Pin PIN_ADC_CV3 = seed::D23;
constexpr Pin PIN_ADC_CV4 = seed::D16;
constexpr Pin PIN_ADC_CV5 = seed::D17;
constexpr Pin PIN_ADC_CV6 = seed::D19;

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
    // std::memset(OUT_L, 0, FM_SAMPLE_RATE * sizeof(float));
    // std::memset(OUT_R, 0, FM_SAMPLE_RATE * sizeof(float));
    std::memset(OUT_L, 0, size);
    std::memset(OUT_R, 0, size);

    // one-liner
    // Parameters
    // fm	Handle to an FM synth instance.
    // left	A pointer to buffer representing the left channel.
    // right	A pointer to buffer representing the right channel.
    // frames	The number of frames (left and right samples) to render.
    // ./libfmsynth/docs/html/group__libfmsynthRender.html
    //
    // fmsynth_render(fm, OUT_L, OUT_R, size);

    // to quickly test `callback`, leave just this code that does audio pass-through
    for (size_t i = 0; i < size; i++)
    {
        OUT_L[i] = IN_L[i];
        OUT_R[i] = IN_L[i];
    }
}

/**
 * Global gate input object.
 * Now using D30 gate input assigned by daisy_versio.cpp.
 * https://electro-smith.github.io/libDaisy/classdaisy_1_1_gate_in.html
 */
// GateIn gate_in;

int main(void)
{
    // Initialize Versio hardware and start audio, ADC
    hw.Init();

    hw.seed.PrintLine("hello world");

    /**
     * see daisy_versio.cpp for pin numbers
     * pitch: PIN_ADC_CV0
     */

    // calibrate 1V/Oct input
    // https://electro-smith.github.io/libDaisy/classdaisy_1_1_voct_calibration.html#details
    VoctCalibration cal;
    float v1 = 0.2f; // Ideal CV input for 1V
    float v3 = 0.6f; // Ideal CV input for 3V
    bool isCalibrated = false;
    // Record with ideal values
    isCalibrated = cal.Record(v1, v3);
    // Get the new data
    float scale, offset;
    isCalibrated = cal.GetData(scale, offset);

    // for example, see DaisyExamples/patch_sm/SimpleOscillator/SimpleOscillator.cpp
    basePitch = 0.f;
    finalPitch = 0.f;

    // Initialize FM synth engine
    // BEFORE callback
    fm = fmsynth_new(FM_SAMPLE_RATE, FM_MAX_VOICES);

    hw.StartAudio(callback);
    hw.StartAdc();

    /** Initialize our Gate Input */
    // TODO get correct pin number for e.g. first CV jack
    // gate_in.Init(seed::D11, false);

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
        hw.UpdateExample();         // Control the LED colors using the knobs and gate inputs. also calls hw.tap.Debounce();
        hw.UpdateLeds();

        // tap button may be set to MOMENTARY or LATCHING
        if (hw.tap.Pressed())
        {
            hw.seed.PrintLine("button pressed");
        }

        // useful tip: smooth CV processing
        // https://forum.electro-smith.com/t/tips-for-smooth-input-cv-processing/1026/3

        // KNOB_0 1 2 3
        // adapted from ./DaisyExamples/patch_sm/SimpleOscillator/SimpleOscillator.cpp
        basePitch = hw.GetKnobValue(DaisyVersio::KNOB_0);
        float velocityCV = hw.GetKnobValue(DaisyVersio::KNOB_1);
        // TODO check it's the correct pin number for pitch CV input jack
        float pitchCV = hw.seed.adc.Get(PIN_ADC_CV0.pin);
        // `fmap` scales 0..1 float value to provided range
        float pitchValue = fmap(pitchCV, 0.f, 60.f);
        finalPitch =
            basePitch + pitchValue;
        float midiNote = fclamp(finalPitch, 0.f, 127.f);
        float midiVelocity = fmap(velocityCV, 0.f, 127.f);
        float freq = mtof(midiNote);

        fmsynth_set_global_parameter(fm, FMSYNTH_GLOBAL_PARAM_VOLUME, hw.GetKnobValue(DaisyVersio::KNOB_0));
        fmsynth_set_global_parameter(fm, FMSYNTH_GLOBAL_PARAM_LFO_FREQ, hw.GetKnobValue(DaisyVersio::KNOB_1));

        // detect gate on/off and send note on/off
        //
        // TODO needs testing.
        // TODO test if below, directly using `hw.Gate()` (pin seed::D30) provided by DaisyVersio, works?
        // `Trig() ` Returns: True if the GPIO just transitioned.
        // if (gate_in.Trig())
        if (hw.gate.Trig())
        {
            //
            // Parameters
            // fm	Handle to an FM synth instance.
            // note	Which note to press. Note is parsed using MIDI rules, i.e. note = 69 is A4. Valid range is [0, 127].
            // velocity	Note velocity. Velocity is parsed using MIDI rules. valie range is [0, 127].
            // ./libfmsynth/docs/html/group__libfmsynthControl.html#ga8186cfec9cf2266289358a14ddb24b31
            fmsynth_note_on(fm, midiNote, midiVelocity);
        }
        // if (!gate_in.State())
        if (!hw.gate.State())
        {
            fmsynth_release_all(fm);
        }
    }
}
