const AWS = require('aws-sdk')
const ssm = new AWS.SSM();

module.exports = async (pathName) => {
  const path = `/smart-dog-collar-test/${pathName}/`
  return await ssm.getParametersByPath({
    Path: path,
    WithDecryption: true
  }).promise()
  .then(data => {
    const params = {}
    data.Parameters.forEach(param => {
      const name = param.Name.replace(path, '')
      params[name] = param.Value
    })
    return params
  })
}