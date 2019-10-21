using_task_library 'camera_rtsp_gstreamer'

Syskit.conf.use_deployment OroGen.camera_rtsp_gstreamer.ReceiverTask => 'receiver'#, valgrind: true

Robot.controller do
    Roby.plan.add_mission_task OroGen.camera_rtsp_gstreamer.ReceiverTask
end
