#pragma once

#include <functional>
#include <utility>
#include <array>

#include <juce_audio_basics/juce_audio_basics.h>
using namespace juce;

//A class that filters an incoming MIDI buffer into a buffer of MIDI messages and a buffer of MPE messages
class MidiMpeSeperator
{
public:
    //clear the buffers, reset the state
    void reset() {
        midiBuffer.clear();
        mpeBuffer.clear();
        activeMPEChannels = std::array<size_t, 16>{0};
    }
    
    //pre-allocate space for the output buffers.
    void prepare(double, int blockSize) {
        midiBuffer.ensureSize(sizeof(MidiMessage)*static_cast<size_t>(blockSize));
        mpeBuffer.ensureSize(sizeof(MidiMessage)*static_cast<size_t>(blockSize));
        reset();
    }
    
    void addEventsToBuffer(MidiBuffer& bufferToAddTo,
                           MidiBuffer::Iterator& iter,
                           const MidiMessage& endMessage){
        MidiMessage currentMessage{};
        int currentPosition{};
        //an equality comparison for MidiMessages
        const auto sameMessage = [](const MidiMessage& message1, const MidiMessage& message2){
            if(message1.getRawDataSize() != message2.getRawDataSize())
                return false;
            else
                for(int i = 0; i < message1.getRawDataSize(); ++i)
                    if(message1.getRawData()[i] != message2.getRawData()[i])
                        return false;
            return true;
        };
        //go through the buffer...
        while (iter.getNextEvent(currentMessage, currentPosition)) {
            //if the message is a pedal, status, or meta message, send it to both buffers
            if(isUniversalMessage(currentMessage)){
                midiBuffer.addEvent(currentMessage, currentPosition);
                mpeBuffer.addEvent(currentMessage, currentPosition);
            }
            //if it's a mpe note off, decrement the number of notes active in the channel
            else if(isMPENoteOff(currentMessage)){
                --activeMPEChannels[static_cast<size_t>(currentMessage.getChannel()-1)];
                mpeBuffer.addEvent(currentMessage, currentPosition);
                break;
            }
            //otherwise, nothing special to do
            else
                bufferToAddTo.addEvent(currentMessage, currentPosition);
            //if we've found the message we want to end on, return
            if(sameMessage(currentMessage, endMessage))
                return;
        }
    }
    
    //takes in a MIDI buffer and returns two MIDI buffers, one for MIDI and one for MPE
    const std::pair<MidiBuffer&, MidiBuffer&> separateBuffers(const MidiBuffer& buffer) {
        //remove old messages from the midi buffers
        midiBuffer.clear();
        mpeBuffer.clear();
        
        //if the buffer is empty, we're done!
        if(!buffer.isEmpty()) {
            MidiBuffer::Iterator iter{buffer};
            MidiMessage message{};
            int samplePosition = 0;
            //Go through the buffer and sort messages into the midi, mpe, or both buffers
            while(iter.getNextEvent(message, samplePosition)) {
                //if this is a status or a pedal message, put it in both buffers
                if(isUniversalMessage(message)){
                    midiBuffer.addEvent(message, samplePosition);
                    mpeBuffer.addEvent(message, samplePosition);
                }
                //If it's a note on in an MPE channel, put it in the mpe buffer
                else if(isMPENoteOn(message)) {
                    mpeBuffer.addEvent(message, samplePosition);
                    ++activeMPEChannels[static_cast<size_t>(message.getChannel()-1)];
                }
                // if the message could be an epxresssion for an MPE note, but no note exists,
                // keep checking subsequent messages to see if this is followed by an MPE note
                // if it is, route all the messages to the MPE buffer, otherwise, the MIDI buffer
                else if(!isInMPEChannel(message) && isMPEExpression(message)) {
                    MidiMessage tempMessage{message};
                    auto tempIter = iter;
                    auto tempPosition = samplePosition;
                    //while there are still events in the buffer
                    bool bufferFinished = false;
                    while (!(bufferFinished = !tempIter.getNextEvent(tempMessage, tempPosition))) {
                        //if the event is an MPE note on, add all events from when we started searching until the note on into the MPE buffer
                        if(isMPEExpression(tempMessage)) {
                            primedMPEChannels[tempMessage.getChannel()-1] = true;
                        }
                        else if (isMPENoteOn(tempMessage)){
                            ++activeMPEChannels[static_cast<size_t>(message.getChannel()-1)];
                            mpeBuffer.addEvent(message, samplePosition);
                            addEventsToBuffer(mpeBuffer, iter, tempMessage);
                            break;
                        }
                        //else if it's not an MPE note or expression, it must be a midi event. Add all of the events to the midiBuffer
                        else
                        {
                            midiBuffer.addEvent(message, samplePosition);
                            addEventsToBuffer(midiBuffer, iter, tempMessage);
                            break;
                        }
                    }
                    // if we've gone through the entire buffer and found no MPE notes
                    // then these are all MIDI messages
                    if(bufferFinished){
                        midiBuffer.addEvent(message, samplePosition);
                        addEventsToBuffer(midiBuffer, iter, tempMessage);
                    }
                }
                // if the message is in a channel with an MPE note,
                // and it is a note off or an MPE expression,
                // add it to the MPE buffer
                else if(isMPEExpression(message) && isInMPEChannel(message))
                    mpeBuffer.addEvent(message, samplePosition);
                else if (isMPENoteOff(message)) {
                    mpeBuffer.addEvent(message, samplePosition);
                    --activeMPEChannels[static_cast<size_t>(message.getChannel()-1)];
                }
                //otherwise, it's a MIDI message.
                else
                    midiBuffer.addEvent(message, samplePosition);
            }
        }
        //All MPE notes have either happened or not, so un-prime all the midi channels
        std::fill(primedMPEChannels.begin(), primedMPEChannels.end(), false);
        return {midiBuffer, mpeBuffer};
    }
    
    // MPE note ons are preceeded by or possibly immediatly followed by MPE expressions.
    // Check to see if the current channel is primed
    // (meaning it has received an mpe expression but has no note)
    // if it has, this is an mpe note
    bool isMPENoteOn(const MidiMessage& message) {
        return message.isNoteOn() && primedMPEChannels[message.getChannel()-1];
    }
    
    bool isMPENoteOff(const MidiMessage& message) {
        return message.isNoteOff() && isInMPEChannel(message);
    }
    
    bool isInMPEChannel(const MidiMessage& message) {
        return activeMPEChannels[static_cast<size_t>(message.getChannel()-1)] > 0;
    }
    
    static bool detectMPEPitchChange(const MidiMessage& message) {
        return message.isPitchWheel();
    }
    
    static bool detectMPEPressure(const MidiMessage& message) {
        return message.isChannelPressure();
    }
    
    static bool detectMPESlide(const MidiMessage& message) {
        if (message.isController())
            return message.getControllerNumber() == 74;
        return false;
    }
    
    static bool isMPEExpression(const MidiMessage& message) {
        return detectMPEPitchChange(message) || detectMPEPressure(message) || detectMPESlide(message);
    }
    
    static bool isPedalMessage(const MidiMessage& message) {
        return message.isSustainPedalOn() || message.isSustainPedalOff() ||
               message.isSoftPedalOn() || message.isSoftPedalOff() ||
               message.isSostenutoPedalOn() || message.isSostenutoPedalOff();
    }
    
    //send any messages that don't have anything to do with controllers, notes, pitchbend, or pressure to both buffers
    bool isUniversalMessage(const MidiMessage& message) {
//      needed for the isMidiMachineControl :/
        int dummyInt{};
        return isPedalMessage(message) ||
               message.isProgramChange () ||
               message.isSysEx() ||
               message.isAftertouch() || //?
               message.isAllNotesOff() ||
               message.isAllSoundOff() ||
               message.isResetAllControllers() ||
               //maybe these?
               message.isMetaEvent() ||
               message.isActiveSense() ||
               message.isMidiStart() ||
               message.isMidiStop() ||
               message.isMidiContinue() ||
               message.isMidiClock() ||
               message.isSongPositionPointer() ||
               message.isQuarterFrame() ||
               message.isFullFrame() ||
               message.isMidiMachineControlMessage() ||
               message.isMidiMachineControlGoto(dummyInt,dummyInt,dummyInt,dummyInt);
        
        
    }
    
private:
    MidiBuffer midiBuffer{};
    MidiBuffer mpeBuffer{};
    
    //holds the amount of active mpe notes in each channel
    std::array<size_t, 16> activeMPEChannels{0};
    //channels that have received an MPE expression but not an MPE note this buffer
    std::array<bool, 16> primedMPEChannels{false};
};
