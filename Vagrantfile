# ===========
# SAP Dissector Plugin for Wireshark
#
# Copyright (C) 2012-2016 by Martin Gallo, Core Security
#
# The plugin was designed and developed by Martin Gallo from the Security
# Consulting Services team of Core Security.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# ==============

Vagrant.configure(2) do |config|

  config.vm.box = "ubuntu/xenial64"

  # Permit X11 forwarding so running the graphical Wireshark works
  config.ssh.forward_x11 = true

  # Define a Vagrant VM to compile as a standalone plugin
  config.vm.define "standalone" do |standalone|
    standalone.vm.provision "shell",
      path: "tools/ubuntu-provision-standalone.sh",
      privileged: false,
      env: {"WIRESHARK_BRANCH" => "master-2.0",
            "PLUGIN_DIR" => "/vagrant/"}

    standalone.vm.provision "shell",
      path: "tools/ubuntu-build-standalone.sh",
      privileged: false,
      env: {"WIRESHARK_BRANCH" => "master-2.0",
            "PLUGIN_DIR" => "/vagrant/"}
  end

  # Define a Vagrant VM to compile from source
  config.vm.define "source" do |source|

    source.vm.provision "shell",
      path: "tools/ubuntu-provision-source.sh",
      privileged: false,
      env: {"WIRESHARK_BRANCH" => "master-2.0",
            "PLUGIN_DIR" => "/vagrant/"}

    source.vm.provision "shell",
      path: "tools/ubuntu-build-source.sh",
      privileged: false,
      env: {"BUILD_WIRESHARK" => "yes",
            "WIRESHARK_BRANCH" => "master-2.0",
            "PLUGIN_DIR" => "/vagrant/"}

    # Add some memory
    source.vm.provider "virtualbox" do |v|
      v.memory = 2048
      v.cpus = 2
    end

  end

end
