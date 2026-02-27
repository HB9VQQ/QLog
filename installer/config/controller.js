function Controller() {
    installer.installationStarted.connect(function() {
        if (systemInfo.productType === "windows") {
            var result = installer.execute("tasklist", ["/FI", "IMAGENAME eq qlog.exe", "/NH"]);
            if (result[0] && result[0].indexOf("qlog.exe") !== -1) {
                QMessageBox.warning("closeQLog", "QLog is running",
                    "Please close QLog before continuing the installation.\n\nClick OK after closing QLog.",
                    QMessageBox.OK);
            }
        }
    });
}
