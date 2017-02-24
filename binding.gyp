{
    "targets": [
        {
            "target_name": "addon",
            "sources": ["native/mmap_file.cpp", "native/deb_read_nan.cpp",
                        "native/addon.cpp"],
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ],
            "libraries": [
                "-larchive"
            ]
        }
    ]
}
