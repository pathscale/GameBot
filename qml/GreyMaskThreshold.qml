import QtQuick 2.0

ShaderEffect {
    Item {
        id: empty
    }
    property variant src: empty
    property variant thr: 0
    fragmentShader: "
        varying highp vec2 qt_TexCoord0;
        uniform sampler2D src;
        uniform float thr;
        void main() {
            lowp vec4 tex = texture2D(src, qt_TexCoord0);
            if (tex.r <= thr) {
                gl_FragColor = vec4(0, 0, 0, 0);
            } else {
                gl_FragColor = vec4(1, 1, 1, 1);
            }
        }
    "
}
