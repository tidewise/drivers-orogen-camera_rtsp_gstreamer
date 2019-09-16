#ifndef camera_rtsp_gstreamer_TYPES_HPP
#define camera_rtsp_gstreamer_TYPES_HPP

#include <string>

/* If you need to define types specific to your oroGen components, define them
 * here. Required headers must be included explicitly
 *
 * However, it is common that you will only import types from your library, in
 * which case you do not need this file
 */

namespace camera_rtsp_gstreamer {
    enum PREDEFINED_ENCODER {
        VP8, VAAPI_VP8, CUSTOM_ENCODING
    };

    /** Definition of the underlying encoder to be used in the webrtc connections */
    struct Encoding {
        /** A predefined encoder
         *
         * Set to CUSTOM_ENCODING to use the raw definition fields
         */
        PREDEFINED_ENCODER encoder = VP8;
        /** Definition of the GStreamer element that will do the encoding
         *
         * Only used if encoder is CUSTOM_ENCODING
         */
        std::string encoder_element;
        /** Definition of the GStreamer element that will do the framing
         *
         * Only used if encoder is CUSTOM_ENCODING
         */
        std::string payload_element;
        /** Definition of the encoder name as passed to the RTP definition
         *
         * Only used if encoder is CUSTOM_ENCODING
         */
        std::string encoder_name;
    };
}

#endif

