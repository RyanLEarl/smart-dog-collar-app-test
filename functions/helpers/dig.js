module.exports = (...args) =>
  args[0]
    ? args.reduce(
        (obj, arg) =>
          (typeof arg === 'function' && obj ? arg(obj) : obj[arg]) || false
      )
    : false;