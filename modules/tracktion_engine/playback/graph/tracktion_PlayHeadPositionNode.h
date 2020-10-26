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

/**
    A Node that calculates a position to show visually what time is currently being processed by the graph based on its internal latency.
*/
class PlayHeadPositionNode final    : public tracktion_graph::Node,
                                      public TracktionEngineNode
{
public:
    PlayHeadPositionNode (ProcessState& processState, std::unique_ptr<tracktion_graph::Node> inputNode,
                          std::atomic<double>& playHeadTimeToUpdate)
        : TracktionEngineNode (processState),
          input (std::move (inputNode)),
          playHeadTime (playHeadTimeToUpdate)
    {
    }
    
    tracktion_graph::NodeProperties getNodeProperties() override
    {
        auto props = input->getNodeProperties();
        
        constexpr size_t playHeadPositionNodeMagicHash = 0x706c617948656164;
        
        if (props.nodeID != 0)
            tracktion_graph::hash_combine (props.nodeID, playHeadPositionNodeMagicHash);

        return props;
    }
    
    std::vector<tracktion_graph::Node*> getDirectInputNodes() override  { return { input.get() }; }
    bool isReadyToProcess() override                                    { return input->hasProcessed(); }
        
    void prepareToPlay (const tracktion_graph::PlaybackInitialisationInfo& info) override
    {
        latencyNumSamples = info.rootNode.getNodeProperties().latencyNumSamples;
        
        // Member variables have to be updated from the previous Node or if the graph gets
        // rebuilt during the countdown period, the playhead time will jump back
        updateFromPreviousNode (info.rootNodeToReplace);
    }
    
    void process (const ProcessContext& pc) override
    {
        // Copy the input buffers to the outputs
        auto sourceBuffers = input->getProcessedOutput();
        jassert (sourceBuffers.audio.getNumChannels() == pc.buffers.audio.getNumChannels());

        pc.buffers.midi.copyFrom (sourceBuffers.midi);
        pc.buffers.audio.copyFrom (sourceBuffers.audio);
        
        updatePlayHeadTime (pc.referenceSampleRange.getLength());
    }

private:
    std::unique_ptr<tracktion_graph::Node> input;
    std::atomic<double>& playHeadTime;

    int latencyNumSamples = 0;
    bool updateReferencePositionOnJump = true;
    
    int64_t numLatencySamplesToCountDown = 0;
    int64_t lastReferenceSamplePosition = 0;
    int64_t referencePositionOnJump = 0;
    
    void updatePlayHeadTime (int64_t numSamples)
    {
        int64_t referenceSamplePosition = getReferenceSampleRange().getStart();
        
        if (getPlayHeadState().didPlayheadJump() || updateReferencePositionOnJump)
        {
            numLatencySamplesToCountDown = latencyNumSamples;
            referencePositionOnJump = updateReferencePositionOnJump ? referenceSamplePosition - numSamples
                                                                    : lastReferenceSamplePosition;
            // TODO: Need to determine why the last position is the "correct" visual position
        }
        
        lastReferenceSamplePosition = referenceSamplePosition;

        if (numLatencySamplesToCountDown > 0)
        {
            const int64_t numSamplesToDecrement = std::min (numLatencySamplesToCountDown, numSamples);
            numLatencySamplesToCountDown -= numSamplesToDecrement;
        }

        if (getPlayHead().isPlaying() && numLatencySamplesToCountDown <= 0)
        {
            referenceSamplePosition -= latencyNumSamples;
        }
        else
        {
            referenceSamplePosition = referencePositionOnJump;
        }

        const int64_t timelinePosition = getPlayHead().referenceSamplePositionToTimelinePosition (referenceSamplePosition);
        const double time = tracktion_graph::sampleToTime (timelinePosition, getSampleRate());

        playHeadTime = time;
        updateReferencePositionOnJump = false;
    }

    void updateFromPreviousNode (Node* rootNodeToReplace)
    {
        if (rootNodeToReplace == nullptr)
            return;
        
        auto nodeIDToLookFor = getNodeProperties().nodeID;
        
        if (nodeIDToLookFor == 0)
            return;

        auto visitor = [this, nodeIDToLookFor] (Node& node)
        {
            if (auto other = dynamic_cast<PlayHeadPositionNode*> (&node))
            {
                if (other->getNodeProperties().nodeID == nodeIDToLookFor)
                {
                    numLatencySamplesToCountDown = other->numLatencySamplesToCountDown;
                    lastReferenceSamplePosition = other->lastReferenceSamplePosition;
                    referencePositionOnJump = other->referencePositionOnJump;
                    updateReferencePositionOnJump = false;
                }
            }
        };
        visitNodes (*rootNodeToReplace, visitor, true);
    }
};

} // namespace tracktion_engine