#include <catch2/catch.hpp>

#include "../Source/Utilities/MidiMPESeparator.h"

// A set of tests making sure that the MIDI/MPE separator can separate a buffer properly


//Make sure a buffer of MIDI messages are routed correctly
TEST_CASE("Midi Messages", "[Separator]") {
	MidiMpeSeperator separator{};
	MidiBuffer buffer;
	separator.prepare(0.0, 6);
	buffer.addEvent(MidiMessage::noteOn (1, 60, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOn (1, 60, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOn (1, 60, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOff(1, 60, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOff(1, 60, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOff(1, 60, 0.5f), 0);

	const auto& [midiBuffer, mpeBuffer] = separator.separateBuffers(buffer);

	REQUIRE(midiBuffer.getNumEvents() == 6);
	REQUIRE(mpeBuffer.isEmpty());

	REQUIRE(midiBuffer.data == buffer.data);
}

//Make sure a buffer of MPE messages are routed correctly
TEST_CASE("MPE Messages", "[Separator]") {
	MidiMpeSeperator separator{};
	MidiBuffer buffer;
	separator.prepare(0.0, 18);
	
	buffer.addEvent(MidiMessage::pitchWheel (3, 0), 0);	
	buffer.addEvent(MidiMessage::channelPressureChange  (3, 64), 0);
	buffer.addEvent(MidiMessage::controllerEvent  (3, 74, 64), 0);
	buffer.addEvent(MidiMessage::pitchWheel (4, 0), 0);
	buffer.addEvent(MidiMessage::noteOn (3, 60, 0.5f), 0);
	buffer.addEvent(MidiMessage::channelPressureChange  (4, 64), 0);
	buffer.addEvent(MidiMessage::controllerEvent  (4, 74, 64), 0);
	buffer.addEvent(MidiMessage::noteOn (4, 61, 0.5f), 0);
	buffer.addEvent(MidiMessage::pitchWheel (5, 0), 0);
	buffer.addEvent(MidiMessage::channelPressureChange  (5, 64), 0);
	buffer.addEvent(MidiMessage::controllerEvent  (5, 74, 64), 0);
	buffer.addEvent(MidiMessage::pitchWheel (3, 0), 0);
	buffer.addEvent(MidiMessage::channelPressureChange  (3, 64), 0);
	buffer.addEvent(MidiMessage::controllerEvent  (3, 74, 64), 0);
	buffer.addEvent(MidiMessage::noteOn (5, 62, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOff (3, 60, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOff (4, 61, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOff (5, 62, 0.5f), 0);
	const auto& [midiBuffer, mpeBuffer] = separator.separateBuffers(buffer);
	REQUIRE(midiBuffer.isEmpty());
	REQUIRE(mpeBuffer.getNumEvents() == 18);

	MidiBuffer::Iterator iterator{mpeBuffer};
	MidiMessage message{};
	int dummy{};

	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isPitchWheel());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isChannelPressure());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isController());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isPitchWheel());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOn());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isChannelPressure());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isController());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOn());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isPitchWheel());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isChannelPressure());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isController());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isPitchWheel());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isChannelPressure());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isController());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOn());
	
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOff());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOff());
	iterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOff());
}

//Make sure a buffer of midi and mpe messages are routed correctly
TEST_CASE("Mixed Messages", "[Separator]") {
	MidiMpeSeperator separator{};
	MidiBuffer buffer;
	separator.prepare(0.0, 24);

	buffer.addEvent(MidiMessage::pitchWheel (3, 0), 0);
	buffer.addEvent(MidiMessage::channelPressureChange  (3, 64), 0);
	buffer.addEvent(MidiMessage::controllerEvent  (3, 74, 64), 0);
	buffer.addEvent(MidiMessage::pitchWheel (4, 0), 0);
	buffer.addEvent(MidiMessage::noteOn (3, 60, 0.5f), 0);
	buffer.addEvent(MidiMessage::channelPressureChange  (4, 64), 0);
	buffer.addEvent(MidiMessage::controllerEvent  (4, 74, 64), 0);
	buffer.addEvent(MidiMessage::noteOn (4, 61, 0.5f), 0);
	buffer.addEvent(MidiMessage::pitchWheel (5, 0), 0);
	buffer.addEvent(MidiMessage::channelPressureChange  (5, 64), 0);
	buffer.addEvent(MidiMessage::noteOn (5, 62, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOn (1, 58, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOn (1, 57, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOn (1, 56, 0.5f), 0);
	buffer.addEvent(MidiMessage::controllerEvent  (5, 74, 64), 0);
	buffer.addEvent(MidiMessage::pitchWheel (3, 0), 0);
	buffer.addEvent(MidiMessage::channelPressureChange  (3, 64), 0);
	buffer.addEvent(MidiMessage::controllerEvent  (3, 74, 64), 0);
	buffer.addEvent(MidiMessage::noteOff(1, 58, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOff (3, 60, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOff (4, 61, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOff(1, 56, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOff (5, 62, 0.5f), 0);
	buffer.addEvent(MidiMessage::noteOff(1, 57, 0.5f), 0);
	const auto& [midiBuffer, mpeBuffer] = separator.separateBuffers(buffer);
	REQUIRE(midiBuffer.getNumEvents() == 6);
	REQUIRE(mpeBuffer.getNumEvents() == 18);

	MidiBuffer::Iterator mpeIterator{mpeBuffer};

	MidiMessage message{};
	int dummy{};

	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isPitchWheel());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isChannelPressure());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isController());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isPitchWheel());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOn());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isChannelPressure());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isController());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOn());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isPitchWheel());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isChannelPressure());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOn());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isController());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isPitchWheel());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isChannelPressure());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isController());
	
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOff());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOff());
	mpeIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOff());



	MidiBuffer::Iterator midiIterator{midiBuffer};

	midiIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOn());
	midiIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOn());
	midiIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOn());

    midiIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOff());
	midiIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOff());
	midiIterator.getNextEvent(message, dummy);
	REQUIRE(message.isNoteOff());
}
