/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2018
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/


namespace tracktion_engine
{

/** Converts a TimePosition to a BeatPosition given a TempoSequence. */
BeatPosition toBeats (TimePosition, const TempoSequence&);

/** Converts a BeatPosition to a TimePosition given a TempoSequence. */
TimePosition toTime (BeatPosition, const TempoSequence&);

//==============================================================================
/** Converts a TimeRange to a BeatRange given a TempoSequence. */
BeatRange toBeats (TimeRange, const TempoSequence&);

/** Converts a BeatRange to a TimeRange given a TempoSequence. */
TimeRange toTime (BeatRange, const TempoSequence&);


//==============================================================================
//==============================================================================
/**
    Represents a time point in an Edit stored as either time or beats.
    This is basically a variant to simplify APIs that can accept either time base.
*/
struct EditTime
{
    /** Creates an EditTime from a TimePosition. */
    EditTime (TimePosition);

    /** Creates an EditTime from a BeatPosition. */
    EditTime (BeatPosition);

private:
    friend TimePosition toTime (EditTime, const TempoSequence&);
    friend BeatPosition toBeats (EditTime, const TempoSequence&);

    std::variant<TimePosition, BeatPosition> position;
};

//==============================================================================
/** Converts an EditTime to a TimePosition.
    N.B. This may be a slow operation if this was created using a BeatPosition.
*/
TimePosition toTime (EditTime, const TempoSequence&);

/** Converts an EditTime to a BeatPosition.
    N.B. This may be a slow operation if this was created using a TimePosition.
*/
BeatPosition toBeats (EditTime, const TempoSequence&);


//==============================================================================
//==============================================================================
namespace temp
{
/**
    Represents a time range in an Edit stored as either time or beats.
    This is basically a variant to simplify APIs that can accept either time base.
*/
struct EditTimeRange
{
    /** Creates an EditTimeRange from a TimeRange. */
    EditTimeRange (TimeRange);

    /** Creates an EditTimeRange from a BeatRange. */
    EditTimeRange (BeatRange);

private:
    friend TimeRange toTime (EditTimeRange, const TempoSequence&);
    friend BeatRange toBeats (EditTimeRange, const TempoSequence&);

    std::variant<TimeRange, BeatRange> range;
};

/** Converts an EditTimeRange to a TimeRange.
    N.B. This may be a slow operation if this was created using a BeatRange.
*/
TimeRange toTime (EditTimeRange, const TempoSequence&);

/** Converts an EditTimeRange to a BeatRange.
    N.B. This may be a slow operation if this was created using a TimeRange.
*/
BeatRange toBeats (EditTimeRange, const TempoSequence&);
}


//==============================================================================
//==============================================================================
} // namespace tracktion_engine

namespace juce
{
    using TimePosition = tracktion_graph::TimePosition;
    using TimeDuration = tracktion_graph::TimeDuration;
    using BeatPosition = tracktion_graph::BeatPosition;
    using BeatDuration = tracktion_graph::BeatDuration;

    template<>
    struct VariantConverter<TimePosition>
    {
        static TimePosition fromVar (const var& v)   { return TimePosition::fromSeconds (static_cast<double> (v)); }
        static var toVar (TimePosition v)            { return v.inSeconds(); }
    };

    template<>
    struct VariantConverter<TimeDuration>
    {
        static TimeDuration fromVar (const var& v)   { return TimeDuration::fromSeconds (static_cast<double> (v)); }
        static var toVar (TimeDuration v)            { return v.inSeconds(); }
    };

    template<>
    struct VariantConverter<BeatPosition>
    {
        static BeatPosition fromVar (const var& v)   { return BeatPosition::fromBeats (static_cast<double> (v)); }
        static var toVar (BeatPosition v)            { return v.inBeats(); }
    };

    template<>
    struct VariantConverter<BeatDuration>
    {
        static BeatDuration fromVar (const var& v)   { return BeatDuration::fromBeats (static_cast<double> (v)); }
        static var toVar (BeatDuration v)            { return v.inBeats(); }
    };
}

namespace tracktion_engine {

//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

inline EditTime::EditTime (TimePosition tp)
    : position (tp)
{
}

inline EditTime::EditTime (BeatPosition bp)
    : position (bp)
{
}

inline TimePosition toTime (EditTime et, const TempoSequence& ts)
{
    // N.B. std::get unavailable prior to macOS 10.14
    if (const auto tp = std::get_if<TimePosition> (&et.position))
        return *tp;

    return tracktion_engine::toTime (*std::get_if<BeatPosition> (&et.position), ts);
}

inline BeatPosition toBeats (EditTime et, const TempoSequence& ts)
{
    if (const auto bp = std::get_if<BeatPosition> (&et.position))
        return *bp;

    return tracktion_engine::toBeats (*std::get_if<TimePosition> (&et.position), ts);
}

namespace temp
{
inline EditTimeRange::EditTimeRange (TimeRange r)
    : range (r)
{
}

inline EditTimeRange::EditTimeRange (BeatRange r)
    : range (r)
{
}

inline TimeRange toTime (EditTimeRange r, const TempoSequence& ts)
{
    // N.B. std::get unavailable prior to macOS 10.14
    if (const auto tr = std::get_if<TimeRange> (&r.range))
        return *tr;

    return tracktion_engine::toTime (*std::get_if<BeatRange> (&r.range), ts);
}

inline BeatRange toBeats (EditTimeRange r, const TempoSequence& ts)
{
    if (const auto br = std::get_if<BeatRange> (&r.range))
        return *br;

    return tracktion_engine::toBeats (*std::get_if<TimeRange> (&r.range), ts);
}
}

} // namespace tracktion_engine
