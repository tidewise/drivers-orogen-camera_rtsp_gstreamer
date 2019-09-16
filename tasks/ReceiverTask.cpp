/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "ReceiverTask.hpp"

using namespace camera_rtsp_gstreamer;

ReceiverTask::ReceiverTask(std::string const& name)
    : ReceiverTaskBase(name)
{
}

ReceiverTask::~ReceiverTask()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See ReceiverTask.hpp for more detailed
// documentation about them.

bool ReceiverTask::configureHook()
{
    if (! ReceiverTaskBase::configureHook())
        return false;
    return true;
}
bool ReceiverTask::startHook()
{
    if (! ReceiverTaskBase::startHook())
        return false;
    return true;
}
void ReceiverTask::updateHook()
{
    ReceiverTaskBase::updateHook();
}
void ReceiverTask::errorHook()
{
    ReceiverTaskBase::errorHook();
}
void ReceiverTask::stopHook()
{
    ReceiverTaskBase::stopHook();
}
void ReceiverTask::cleanupHook()
{
    ReceiverTaskBase::cleanupHook();
}
