const aws = require('aws-sdk')
const stepfunctions = new aws.StepFunctions({ region: 'us-west-2' })

exports.handler = async (event, context, callback) => {
  console.log('SMART DOG COLLAR INVOKED: ', JSON.stringify(event));
  const record = event.Records[0];
  //const { serviceType } = JSON.parse(record.body);
  const stateMachineArn = process.env.stateMachineArn;
  const params = { stateMachineArn, input: record.body };
  console.log('INVOKING STEP', record.body);
  await stepfunctions.startExecution(params).promise()
    .then(() => console.log('started execution of step function'))
    .catch(() => console.log('err while executing step function'))
  return Promise.resolve();
}