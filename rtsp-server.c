#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

static gboolean
timeout (GstRTSPServer * server)
{
  GstRTSPSessionPool *pool;

  pool = gst_rtsp_server_get_session_pool (server);
  gst_rtsp_session_pool_cleanup (pool);
  g_object_unref (pool);

  return TRUE;
}

int
main (int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory_hd, *factory_sd;

  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);
  server = gst_rtsp_server_new ();
  mounts = gst_rtsp_server_get_mount_points (server);

  factory_hd = gst_rtsp_media_factory_new ();
  gst_rtsp_media_factory_set_launch (factory_hd, "( "
      "v4l2src device=/dev/video0 ! "
      "video/x-raw,width=1280,height=720 !"
      "x264enc speed-preset=ultrafast psy-tune=grain tune=zerolatency bitrate=1536 ! "
      "h264parse ! "
      "queue ! "
      "rtph264pay name=pay0 pt=96 "
      "alsasrc device=plughw:1 ! "
      "audioconvert ! "
      "audio/x-raw,rate=44100,channels=1,depth=16 ! "
      "voaacenc bitrate=64000 ! "
      "aacparse ! "
      "queue ! "
      "rtpmp4apay name=pay1 pt=97 " ")");
  gst_rtsp_mount_points_add_factory (mounts, "/720p", factory_hd);

  factory_sd = gst_rtsp_media_factory_new ();
  gst_rtsp_media_factory_set_launch (factory_sd, "( "
      "v4l2src device=/dev/video0 ! "
      "video/x-raw,width=640,height=480 !"
      "x264enc speed-preset=ultrafast psy-tune=grain tune=zerolatency bitrate=836 ! "
      "h264parse ! "
      "queue ! "
      "rtph264pay name=pay0 pt=96 "
      "alsasrc device=plughw:1 ! "
      "audioconvert ! "
      "audio/x-raw,rate=44100,channels=1,depth=16 ! "
      "voaacenc bitrate=64000 ! "
      "aacparse ! "
      "queue ! "
      "rtpmp4apay name=pay1 pt=97 " ")");
  gst_rtsp_mount_points_add_factory (mounts, "/480p", factory_sd);

  g_object_unref (mounts);
  if (gst_rtsp_server_attach (server, NULL) == 0)
    goto failed;
  g_timeout_add_seconds (2, (GSourceFunc) timeout, server);

  g_print ("stream ready at rtsp://[ip]:8554/[720p | 480p]\n");
  g_main_loop_run (loop);

  return 0;
failed:
  {
    g_print ("failed to attach the server\n");
    return -1;
  }
}
