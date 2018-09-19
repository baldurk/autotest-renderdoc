import renderdoc

# Alias for convenience - we need to import as-is so types don't get confused
rd = renderdoc


def open_capture(filename="", cap: rd.CaptureFile=None):
    """
    Opens a capture file and begins a replay.

    :param filename: The filename to open, or empty if cap is used.
    :param cap: The capture file to use, or ``None`` if a filename is given.
    :return: A replay controller for the capture
    :rtype: renderdoc.ReplayController
    """

    # Open a capture file handle
    own_cap = False

    if cap is None:
        own_cap = True

        cap = rd.OpenCaptureFile()

        # Open a particular file
        status = cap.OpenFile(filename, '', None)

        # Make sure the file opened successfully
        if status != rd.ReplayStatus.Succeeded:
            cap.Shutdown()
            raise RuntimeError("Couldn't open '{}': {}".format(filename, str(status)))

        # Make sure we can replay
        if not cap.LocalReplaySupport():
            cap.Shutdown()
            raise RuntimeError("Capture cannot be replayed")

    status, controller = cap.OpenCapture(None)

    if own_cap:
        cap.Shutdown()

    if status != rd.ReplayStatus.Succeeded:
        raise RuntimeError("Couldn't initialise replay: {}".format(str(status)))

    return controller
