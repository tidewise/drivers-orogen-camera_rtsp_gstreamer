/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

// TODO: See pipeline conversion to RGB format and set an Frame object with the image

#include "ReceiverTask.hpp"
#include <memory>

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

    cout << "1 abacaxi" << endl;

    // Camera Onvif Initialization and setting
    unique_ptr<camera_onvif::CameraOnvif> camera(new camera_onvif::CameraOnvif(_user.get(), _pass.get(), _ip.get()));
    camera->setResolution(_width.get(), _height.get());

    cout << "2 abacaxi" << endl;

    auto params = camera_onvif::ImageParam();
    params.brightness = _brightness.get();
    params.color_saturation = _color_saturation.get();
    params.contrast = _contrast.get();
    camera->setImageParam(params);

    cout << "3 abacaxi" << endl;

    GError *error = NULL;
    gchar *descr = g_strdup_printf ("rtspsrc location=rtsp://%s:%s@%s:554 "
        "drop-on-latency=true latency=0 buffer-mode=auto ! rtph264depay ! h264parse "
        "! vaapih264dec low-latency=true ! vaapipostproc ! video/x-raw,format=(string)RGBA !"
        " appsink name=sink max-buffers=1 drop=TRUE",
         _user.get().c_str(), _pass.get().c_str(), _ip.get().c_str());

    cout << "3.5 abacaxi" << endl;

    unique_ptr<CustomData> data(new CustomData());

    cout << "3.75 abacaxi" << endl;

    data->pipeline = gst_parse_launch (descr, &error);

    cout << "4 abacaxi" << endl;

    if (error != NULL) {
        g_print ("could not construct pipeline: %s\n", error->message);
        g_clear_error (&error);
        return false;
    }

    cout << "5 abacaxi" << endl;

    data->sink = gst_bin_get_by_name (GST_BIN (data->pipeline), "sink");

    g_object_set (data->sink, "emit-signals", TRUE, "sync", FALSE, NULL);
    g_signal_connect (data->sink, "new-sample", G_CALLBACK (new_sample), &data);

    cout << "6 abacaxi" << endl;

    data->writer = &_images;

    /* Initializing Rock frame */
    data->frame = new Frame(_width.get(), _height.get(), 8, frame_mode_t::MODE_RGB);

    cout << "7 abacaxi" << endl;

    // Check if the resources are available and alocate it
    GstStateChangeReturn ret = gst_element_set_state (data->pipeline, GST_STATE_READY);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the ready state.\n");
        return false;
    }

    cout << "8 abacaxi" << endl;

    m_data = data.release();
    m_camera = camera.release();

    cout << "9 abacaxi" << endl;

    return true;
}
bool ReceiverTask::startHook()
{
    if (! ReceiverTaskBase::startHook())
        return false;

    cout << "1 mamao" << endl;

    /* Start playing */
    GstStateChangeReturn ret = gst_element_set_state (m_data->pipeline, GST_STATE_PLAYING);

    cout << "2 mamao" << endl;

    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        return false;
    }

    return true;
}
void ReceiverTask::updateHook()
{
    ReceiverTaskBase::updateHook();
    cout << "1 update" << endl;
    auto img_param = camera_onvif::ImageParam();
    cout << "2 update" << endl;
    if (_image_param.read(img_param) == RTT::NewData){
        m_camera->setImageParam(img_param);
    }
    cout << "3 update" << endl;
}
void ReceiverTask::errorHook()
{
    ReceiverTaskBase::errorHook();
    cout << "error" << endl;
}
void ReceiverTask::stopHook()
{
    ReceiverTaskBase::stopHook();
    cout << "stop" << endl;
    gst_element_set_state (m_data->pipeline, GST_STATE_PAUSED);
}
void ReceiverTask::cleanupHook()
{
    cout << "cleanup" << endl;
    gst_element_set_state (m_data->pipeline, GST_STATE_NULL);
    ReceiverTaskBase::cleanupHook();
    delete m_camera;
}
