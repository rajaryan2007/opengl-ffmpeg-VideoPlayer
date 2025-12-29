find_package(FFMPEG CONFIG REQUIRED)

target_link_libraries(video_rendering PRIVATE
    FFMPEG::avcodec
    FFMPEG::avformat
    FFMPEG::avutil
)
