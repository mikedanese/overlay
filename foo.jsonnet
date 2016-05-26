{
  local overlay = import "overlay.jsonnet",
  modules: {
    foo: {
    before_actions: {
        b: overlay.host_exec {
          cmd: "ls",
        }
    },
    after_actions: {
        a: overlay.host_exec {
          cmd: "ls",
        }
    },
      actions: {
        loading: overlay.exec {
          cmd: "echo dumdum",
        },
      },
    },
  },
}
