{
    "targets": [
        {
            "target_name": "addon",
            "sources": ["src/mmap_file.cpp", "src/deb_read_nan.cpp",
                        "src/addon.cpp"],
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ],
            "libraries": [
                "-larchive"
            ]
        }
    ]
}
