#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed  seed;
static Biquad     flt;
static Oscillator osc, lfo;


ShiftRegister4021<1> sr_4021;
uint8_t              sr_4021_state_[8];

// SR 4021 with 8 buttons connected to Seed pins 35, 34 and 33 (PA2, PG9, PD11)

void init_SR_4021() 
{
    ShiftRegister4021<1>::Config SR_4021_cfg;
    SR_4021_cfg.clk     = {DSY_GPIOA, 2};  // seed.GetPin(35); // PA2
    SR_4021_cfg.latch   = {DSY_GPIOG, 9};  // seed.GetPin(34); // PG9
    SR_4021_cfg.data[0] = {DSY_GPIOD, 11}; // seed.GetPin(33); // PD11
    sr_4021.Init(SR_4021_cfg);
}

void update_SR_4021() 
{
    sr_4021.Update();
    for(size_t i = 0; i < 8; i++)
    {
        uint8_t idx_, offset_;
        offset_ = i > 7 ? 8 : 0;
        idx_    = (7 - (i % 8)) + offset_;
        sr_4021_state_[idx_]
            = sr_4021.State(i) | (sr_4021_state_[idx_] << 1);
    }
}

bool SR_4021State(size_t idx)
{
    return sr_4021_state_[idx] == 0x00;
}

bool SR_4021RisingEdge(size_t idx)
{
    return sr_4021_state_[idx] == 0x80;
}

bool SR_4021FallingEdge(size_t idx)
{
    return sr_4021_state_[idx] == 0x7F;
}

void ProcessControls();


static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    ProcessControls();

    float saw, freq, output;
    for(size_t i = 0; i < size; i += 2)
    {
        freq = 10000 + (lfo.Process() * 10000);
        saw  = osc.Process();

        flt.SetCutoff(freq);
        output = flt.Process(saw);

        // left out
        out[i] = output;

        // right out
        out[i + 1] = output;
    }
}

int main(void)
{
    // initialize seed hardware and daisysp modules
    float sample_rate;
    seed.Configure();
    seed.Init();
    sample_rate = seed.AudioSampleRate();

    init_SR_4021();

    // initialize Biquad and set parameters
    flt.Init(sample_rate);
    flt.SetRes(0.7);

    // set parameters for sine oscillator object
    lfo.Init(sample_rate);
    lfo.SetWaveform(Oscillator::WAVE_TRI);
    lfo.SetAmp(1);
    lfo.SetFreq(.4);

    // set parameters for sine oscillator object
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc.SetFreq(100);
    osc.SetAmp(0.25);


    // start callback
    seed.StartAudio(AudioCallback);


    while(1) {}
}

void ProcessControls()
{
    update_SR_4021();
    if( SR_4021RisingEdge(0) ) { osc.SetFreq(50); }
    if( SR_4021RisingEdge(1) ) { osc.SetFreq(100); }
    if( SR_4021RisingEdge(2) ) { osc.SetFreq(150); }
    if( SR_4021RisingEdge(3) ) { osc.SetFreq(200); }
    if( SR_4021RisingEdge(4) ) { osc.SetFreq(300); }
    if( SR_4021RisingEdge(5) ) { osc.SetFreq(400); }
    if( SR_4021RisingEdge(6) ) { osc.SetFreq(600); }
    if( SR_4021RisingEdge(7) ) { osc.SetFreq(800); }   
    
}
