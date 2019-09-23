/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

// TODO: See pipeline conversion to RGB format and set an Frame object with the image

#include "ReceiverTask.hpp"

using namespace camera_rtsp_gstreamer;

/* This function will be called by the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) {
  GstPad *sink_pad = gst_element_get_static_pad (data->convert, "sink");
  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;

  g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

  /* If our converter is already linked, we have nothing to do here */
  if (gst_pad_is_linked (sink_pad)) {
    g_print ("We are already linked. Ignoring.\n");
    goto exit;
  }

  /* Check the new pad's type */
  new_pad_caps = gst_pad_get_current_caps (new_pad);
  new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
  new_pad_type = gst_structure_get_name (new_pad_struct);
  if (!g_str_has_prefix (new_pad_type, "video/x-raw")) {
    g_print ("It has type '%s' which is not raw video. Ignoring.\n", new_pad_type);
    goto exit;
  }

  /* Attempt the link */
  ret = gst_pad_link (new_pad, sink_pad);
  if (GST_PAD_LINK_FAILED (ret)) {
    g_print ("Type is '%s' but link failed.\n", new_pad_type);
  } else {
    g_print ("Link succeeded (type '%s').\n", new_pad_type);
  }

exit:
  /* Unreference the new pad's caps, if we got them */
  if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);

  /* Unreference the sink pad */
  gst_object_unref (sink_pad);
}

GstFlowReturn ReceiverTask::new_sample (GstElement *sink, CustomData *data) {
    /* Retrieve the buffer */
    GstSample *sample;
    g_signal_emit_by_name (sink, "pull-sample", &sample);
    if (!sample) {
            return GST_FLOW_OK;
    }


    /* If we have a new sample we have to send it to our Rock frame */
    GstBuffer *buffer = gst_sample_get_buffer (sample);
    if (buffer != NULL){
        GstMemory *memory = gst_buffer_get_memory(buffer, 0);
        GstMapInfo info;
        if (gst_memory_map(memory, &info, GST_MAP_READ)){
            data->frame.setImage(info.data, info.size);
            data->writer->write(data->frame);
        }
        gst_memory_unmap(memory, &info);
        gst_memory_unref (memory);
    }

    gst_sample_unref (sample);
    return GST_FLOW_OK;

}

static void cb_message (GstBus * bus, GstMessage * msg, CustomData * data){
    gboolean terminate = FALSE;
    GError *err;
    gchar *debug_info;
    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ERROR:
            gst_message_parse_error (msg, &err, &debug_info);
            g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
            g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
            g_clear_error (&err);
            g_free (debug_info);
            terminate = TRUE;
            break;
        case GST_MESSAGE_EOS:
            g_print ("End-Of-Stream reached.\n");
            terminate = TRUE;
            break;
        case GST_MESSAGE_STATE_CHANGED:
            /* We are only interested in state-changed messages from the pipeline */
            if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->pipeline)) {
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
            g_print ("Pipeline state changed from %s to %s:\n",
                gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
            }
            break;
        default:
            /* We should not reach here */
            g_printerr ("Unexpected message received.\n");
            break;
    }
    gst_message_unref (msg);
}

ReceiverTask::ReceiverTask(std::string const& name)
    : ReceiverTaskBase(name)
{
    // _uri.set('rtsp://admin:camera01@10.20.0.188:80');
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

    data.source = gst_element_factory_make ("uridecodebin", "source");
    data.convert = gst_element_factory_make ("videoconvert", "convert");
    data.sink = gst_element_factory_make ("appsink", "app_sink");

    /* Create the empty pipeline */
    data.pipeline = gst_pipeline_new ("test-pipeline");

    if (!data.pipeline || !data.source || !data.convert || !data.sink) {
        g_printerr ("Not all elements could be created.\n");
        return false;
    }

    gst_bin_add_many (GST_BIN (data.pipeline), data.source, data.convert , data.sink, NULL);
    if (!gst_element_link (data.convert, data.sink)) {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref (data.pipeline);
        return false;
    }

    data.writer = &_images;

    return true;
}
bool ReceiverTask::startHook()
{
    if (! ReceiverTaskBase::startHook())
        return false;

    GstBus *bus;
    GstStateChangeReturn ret;
    GstCaps *video_caps;
    GstVideoInfo info;

    string uri = _uri.get() + " latency=0 buffer-mode=auto ! decodebin ! videoconvert ! autovideosink sync=false";

    g_object_set (data.source, "uri", uri.c_str(), NULL);

    /* Connect to the pad-added signal */
    g_signal_connect (data.source, "pad-added", G_CALLBACK (pad_added_handler), &data);

    gst_video_info_set_format (&info, GST_VIDEO_FORMAT_RGB, 1920, 1080);
    video_caps = gst_video_info_to_caps (&info);

    g_object_set (data.sink, "emit-signals", TRUE, "sync", FALSE, "caps", video_caps, NULL);
    g_signal_connect (data.sink, "new-sample", G_CALLBACK (new_sample), &data);

    gst_caps_unref (video_caps);

    /* Start playing */
    ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (data.pipeline);
        return false;
    }

    /* Listen to the bus */
    bus = gst_element_get_bus (data.pipeline);
    gst_bus_add_signal_watch (bus);
    g_signal_connect (bus, "message", G_CALLBACK (cb_message), &data);

    /* Setting Rock frame */
    base::samples::frame::frame_mode_t mode;
    mode = base::samples::frame::frame_mode_t::MODE_RGB;
    data.frame = base::samples::frame::Frame(_width.get(), _height.get(), 8, mode);

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
    gst_object_unref (data.pipeline);
}
void ReceiverTask::cleanupHook()
{
    ReceiverTaskBase::cleanupHook();

}
