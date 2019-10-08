/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

// TODO: See pipeline conversion to RGB format and set an Frame object with the image

#include "ReceiverTask.hpp"

using namespace std;
using namespace base::samples::frame;
using namespace camera_rtsp_gstreamer;

GstFlowReturn ReceiverTask::new_sample (GstElement *sink, CustomData *data) {
    /* Retrieve the buffer */
    GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));

    /* If we have a new sample we have to send it to our Rock frame */
    GstBuffer *buffer = gst_sample_get_buffer (sample);
    if (buffer != NULL){
        GstMemory *memory = gst_buffer_get_memory(buffer, 0);
        GstMapInfo info;
        if (gst_memory_map(memory, &info, GST_MAP_READ)) {
            CustomData::Frame* frame = data->frame.write_access();
            frame->time = base::Time::now();
            frame->frame_status = STATUS_VALID;

            uint8_t* pixels = &(frame->image[0]);
            int pixel_count = frame->getPixelCount();
            for (int i = 0; i < pixel_count; ++i) {
                pixels[i * 3] = info.data[i * 4];
                pixels[i * 3 + 1] = info.data[i * 4 + 1];
                pixels[i * 3 + 2] = info.data[i * 4 + 2];
            }
            data->frame.reset(frame);
            data->writer->write(data->frame);
        }
        gst_memory_unmap(memory, &info);
        gst_memory_unref (memory);
    }

    gst_sample_unref (sample);
    return GST_FLOW_OK;

}

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

    GError *error = NULL;
    gchar *descr = g_strdup_printf ("rtspsrc location=%s drop-on-latency=true latency=0 buffer-mode=auto ! rtph264depay ! h264parse ! vaapih264dec low-latency=true ! vaapipostproc ! video/x-raw,format=(string)RGBA !"
      " appsink name=sink max-buffers=1 drop=TRUE", "rtsp://admin:camera01@10.20.0.188:554");//_uri.get().c_str());
    data.pipeline = gst_parse_launch (descr, &error);

    if (error != NULL) {
        g_print ("could not construct pipeline: %s\n", error->message);
        g_clear_error (&error);
        return false;
    }

    data.sink = gst_bin_get_by_name (GST_BIN (data.pipeline), "sink");

    g_object_set (data.sink, "emit-signals", TRUE, "sync", FALSE, NULL);
    g_signal_connect (data.sink, "new-sample", G_CALLBACK (new_sample), &data);

    data.writer = &_images;

    /* Setting Rock frame */
    data.frame = new Frame(_width.get(), _height.get(), 8,
                           frame_mode_t::MODE_RGB);

    return true;
}
bool ReceiverTask::startHook()
{
    if (! ReceiverTaskBase::startHook())
        return false;


    /* Start playing */
    GstStateChangeReturn ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (data.pipeline);
        return false;
    }

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
    gst_element_set_state (data.pipeline, GST_STATE_NULL);
}
void ReceiverTask::cleanupHook()
{
    ReceiverTaskBase::cleanupHook();
    gst_object_unref (data.pipeline);

}
